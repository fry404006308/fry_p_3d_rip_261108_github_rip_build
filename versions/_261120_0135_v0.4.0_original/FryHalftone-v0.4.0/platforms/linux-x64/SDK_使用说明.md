# 水转印加网 SDK 使用说明（给桌面端 / 云服务对接方）

> 包名 `fry-halftone` ｜ 算法 `water_transfer_depth`（水转印 + 深度抖动）｜ 接口：C ABI（`include/fry_halftone.h`）｜ ★引擎：C++（384 起）
>
> 与旧的 lmst（`LMSTYzHalftone`）相比：**接口形状刻意保持一致**（同样的「模型路径 + 输出目录 + 目标高度 + 进度回调 + 错误缓冲」），
> 桌面端照着原来的调用代码改函数名即可；另外多了一个 **JSON 参数接口**，以后加参数不用再改 ABI。

---

## 一、包里有什么

```
fry-halftone/
├── bin/fry_halftone.dll         ← ★ 运行只需要这一个文件（Linux: libfry_halftone.so ｜ macOS: libfry_halftone.dylib）
├── bin/fry-halftone-cli.exe     ← ★409 命令行版（与库同目录；Linux / macOS 无 .exe），见第十节
├── lib/fry_halftone.lib         ← Windows 导入库（C/C++ 隐式链接时用；动态加载不需要）
├── include/fry_halftone.h       ← C 接口（纯 C / C++ / C# P-Invoke / Delphi 都照它写）
├── examples/c/fry_halftone_demo.c          ← 最小可编译示例（C）
├── examples/csharp/FryHalftoneDemo.cs      ← ★393 C# P/Invoke 示例（调用约定 / 字符集 / 回调保活都标了）
├── examples/python/fry_halftone_demo.py    ← ★393 Python ctypes 示例（不装第三方包就能跑）
├── VERSION                      ← 一行版本号
├── sdk_info.json                ← 版本 / 平台 / 引擎 / 算法指纹 / dll 的 sha256 / ★393 集成信息（见第 1.2 节）
├── params_schema.json           ← ★387 参数说明（机读）：dll 自己导出的参数的类型 / 默认值 / 取值范围
├── SDK_参数说明.md              ← ★387 参数说明（人读）：由 params_schema.json 逐行生成，不要手改
├── delivery_manifest.json       ← ★393 交付清单（机读）：这个包**应该**有哪些文件 + 每个文件的 sha256 + 为什么必需
├── verify_delivery.py           ← ★393 自检脚本：`python verify_delivery.py` 一次回答「完整吗 / 能加载吗 / 版本一致吗」
├── SHA256SUMS.txt               ← 全部文件的校验和
├── CHANGELOG.md                 ← 变更历史
├── LICENSE.txt                  ← ★393 许可与第三方声明（本库不链接任何第三方开源库）
└── SDK_使用说明.md（本文件） / SDK_README_EN.md
```

### 1.1 ★ 拿到包先跑一次自检（30 秒）

```
python verify_delivery.py
```

它按 `delivery_manifest.json` 逐项核对（缺件 / 空文件 / 内容被改）、真加载一次 dll、再比对四处版本号
（`VERSION` ｜ `sdk_info.json` ｜ `FryHalftone_GetVersion()` ｜ **Windows 文件属性**）。
全绿就可以直接集成；有红把整段输出发回来即可，不用再描述现象。

### 1.2 ★ 集成前最常问的那些（都写在 `sdk_info.json` 里，不用问人）

| 问题 | 字段 | 本版取值（Windows） |
| --- | --- | --- |
| 什么 ABI、什么调用约定 | `abi_str` | C ABI，`__stdcall`（其它平台默认） |
| 要不要装 VC++ 运行库 | `runtime_str` | **不要**：C++ 运行库静态链接，UCRT 用系统自带 |
| 最低系统 | `min_os_str` | Windows 10 x64 ｜ ★409 Linux / macOS 是从二进制**实测**的（见第八节） |
| 线程安全吗 | `thread_safety_str` | 同一时刻只跑一个作业（并发返回 6 BUSY）；Cancel 可从别的线程调 |
| 内存要多少 | `memory_hint_str` | 随体素数线性增长；17 mm 半球件约 0.83 GB |
| 联网 / 写注册表吗 | `network_bool` / `writes_registry_bool` | 都是 `false` |
| 导出了哪些函数 | `entry_points_list` | 11 个 `FryHalftone_*` |
| 返回码什么意思 | `return_codes_dict` | 0 成功 / 1 失败 / 2 参数 / 5 取消 / 6 忙 / 7 不支持 |
| 版本号在哪几处 | `version_sources_list` | 四处（含**右键属性 → 详细信息**） |

★ **Windows 文件属性**：右键 dll →「属性 → 详细信息」能直接看到**文件版本、产品名、公司、版权**
（★393 起有；这一栏与上面四处版本号由出包闸逐字比对，不一致不许出包）。

| 平台 | 产物文件 | 体积（约） |
| --- | --- | --- |
| win32-x64 | `fry-halftone-<版本>-win32-x64.zip` | 约 **0.5 MB**（dll 约 0.61 MB；382 的 Python 版 96 MB） |
| linux-x64 | `fry-halftone-<版本>-linux-x64.tar.zst` | ★409 实测：prebuilt（zig）**0.60 MB** ｜ native（gcc）1.20 MB |
| darwin-arm64 / darwin-x64 | `fry-halftone-<版本>-darwin-<arm64/x64>.tar.zst` | ★409 在 Mac 上跑验收包后产出（见 `SDK_发布与CI.md`） |

> ⚠ **按 `manifest.json` 的 `file` 字段取文件名**，不要自己拼扩展名（Windows 是 `.zip`，另两个是 `.tar.zst`）。

## 二、五条必须知道的事

1. ★ **算法在你的进程里跑**（384 起是 C++；382/383 是子进程里的 Python）。结果与算法仓 Python 生产线**逐字节一致**。
   半球件（最长边 16.851 mm）约 **3.6 s、峰值约 0.83 GB 内存**（同机 Python 102 s / 11.5 GB）；内存随体素数线性增长（体素数 ≈ 三边 mm ÷ (0.137, 0.035, 0.01) 相乘）。
2. **同一时刻只跑一个作业**：第二个并发调用直接返回 `FRY_HALFTONE_BUSY (6)`。
3. **进度回调在调用 `SliceModel` 的那个线程上同步触发**（回调里别做耗时操作）；要中止，从**另一个线程**调 `FryHalftone_Cancel()`，
   作业在下一个检查点（各步之间、写 BMP 每 32 层）返回 `FRY_HALFTONE_CANCELLED (5)`。
4. **暂不支持的模型返回 `FRY_HALFTONE_UNSUPPORTED (7)`**，errorBuffer 写清是哪一种 —— 绝不静默出一个与 Python 不一致的结果（清单见第六节）。
5. **坏文件返回 `FRY_HALFTONE_FAILED (1)`，不会把你的进程带崩或卡死**（384 用 17.4 万个随机弄坏的 PNG / JPEG / GLB 撞过：第一轮撞出并修掉 1 个死循环，第二轮 0 崩溃 0 卡死）。
   ⚠ 内存耗尽仍会失败（返回 1「内存不足」）；超大件建议先用小尺寸试一次。

## 三、最短调用（C）

```c
#include "fry_halftone.h"

static void FRY_CALL on_progress(int percent, const char* stage, void* user) {
    printf("[%3d%%] %s\n", percent, stage);
}

char err[4096];
int rc = FryHalftone_SliceModelUtf8("D:/models/a.glb", "D:/out/a_slices", 30.0,   /* 成品 z 高度 30 mm（与 lmst 同义） */
                                    on_progress, NULL, err, sizeof(err));
if (rc != FRY_HALFTONE_OK) { /* 读 err */ }
```

Windows 上路径建议用宽字符版 `FryHalftone_SliceModel(const wchar_t* …)`（中文路径已测过）。

## 四、接口一览

| 函数 | 作用 |
| --- | --- |
| `FryHalftone_GetVersion()` | 版本号：★411 交付版是 `X.Y.Z`（0.1.0 起每次交付递增，见第十二节）；本地调试构建是 `0.0.0-dev`（不交付） |
| `FryHalftone_GetAlgorithmName()` | 固定 `water_transfer_depth` |
| `FryHalftone_GetEngine()` | ★384 固定 `cpp` |
| `FryHalftone_GetAlgorithmFingerprint()` | 64 位 sha256：库里编的**到底是哪一版算法源码**（文件名可以错，指纹不会） |
| `FryHalftone_GetParamSchemaJson()` | ★384 参数白名单 JSON，可据此生成设置界面；★387 起（`fry-halftone-params/3`）每个参数还带**默认值与取值范围**，与包里 `params_schema.json` 同一份 |
| `FryHalftone_SliceModel` / `…Utf8` | 模型 → BMP 切片（与 lmst 同形状；其余参数用默认值） |
| `FryHalftone_SliceModelJsonUtf8` | 参数走 JSON，结果回 JSON（推荐新代码用它） |
| `FryHalftone_Cancel()` | 中止正在跑的作业（任意线程调用；没作业时返回 0） |
| `FryHalftone_SetWorkerPathUtf8` / `GetWorkerPathUtf8` | 382/383 子进程时代的函数，保留符号只为 ABI 兼容（设置无效、查询返回空串） |

### 返回码

| 值 | 名字 | 含义 |
| ---: | --- | --- |
| 0 | `FRY_HALFTONE_OK` | 成功 |
| 1 | `FRY_HALFTONE_FAILED` | 跑了但失败了（坏文件 / 内存不足 / 输出宽度小于模型…；读 errorBuffer） |
| 2 | `FRY_HALFTONE_BAD_ARGUMENT` | 参数不合法，一步都没跑 |
| 3 | `FRY_HALFTONE_NO_WORKER` | （382/383 子进程时代；384 起不会再返回） |
| 4 | `FRY_HALFTONE_PROTOCOL` | （382/383 子进程时代；384 起不会再返回） |
| 5 | `FRY_HALFTONE_CANCELLED` | 被 `FryHalftone_Cancel` 中止 |
| 6 | `FRY_HALFTONE_BUSY` | 已经有一个作业在跑 |
| 7 | `FRY_HALFTONE_UNSUPPORTED` | ★384 模型用到了 C++ 版暂不支持的特性（见第六节） |

## 五、JSON 参数接口

```json
{
  "model": "D:/models/a.glb",
  "out": "D:/out/a_slices",
  "params": {
    "target_height_mm": 30.0,
    "ink_c_um": 400, "ink_m_um": 500, "ink_y_um": 300,
    "use_black_k": false,
    "color_thickness_mm": null,
    "band_uninked_fill": "transparent",
    "support_enable": true
  }
}
```

> ★387 **参数的类型 / 默认值 / 取值范围以随包的 `SDK_参数说明.md`（人读）/ `params_schema.json`（机读）为准** ——
> 它们由 dll 自己导出，出包冒烟时真调 dll 核对过：只给尺寸与把默认值全写出来结果逐字节相同；75 个压线值的收 / 拒与说明一致。
> 下表是**带讲解**的版本（为什么这样设计、调参牵动什么），数字若与随包说明不同，说明这份文档没跟上，请以随包说明为准并告诉我们。

| 参数 | JSON 类型 | 默认 | 说明 |
| --- | --- | --- | --- |
| `target_height_mm` | 数字 ≥ 0 或 `null` | **无（尺寸必须给）** | 成品 **z 高度** mm：模型先按包围盒摆正（最短边朝 z）再等比缩放。**与 lmst 的 `targetHeightMm` 同义**；**0 = 保持模型自带尺寸** |
| `target_longest_edge_mm` | 数字 > 0 或 `null` | 无 | 成品**最长边** mm（与算法仓界面 / 流程表 `TARGET_MM` 同义）。**与 `target_height_mm` 二选一，必须给一个** |
| `ink_c_um` / `ink_m_um` / `ink_y_um` | 数字 ≥ 0 | 400 / 500 / 300 | 三色满喷厚度 µm（本机实况） |
| ★`use_black_k` | 布尔 | `false` | `true` ⇒ CMYK 四通道：灰分量 min(C,M,Y) × `black_k_amount` 挪给 K 墨。**本机暂无 K 墨，为将来保留** |
| ★`ink_k_um` | 数字 ≥ 0 或 `null` | `null` | K 满喷厚度 µm；**`use_black_k=true` 时必须给**（不给返回 2） |
| ★`black_k_amount` | 数字，[0, 1] | `1.0` | 灰分量替换强度：0 = 不替换（但通道仍是 4 个、色带容量含 K）；1 = 全部灰分量给 K |
| ★`color_thickness_mm` | 数字 > 0 或 `null` | `null` | 色带厚度 mm；**`null` = 各墨满喷厚度之和**（C+M+Y[+K]，默认 1.2 mm）。调小 ⇒ 墨往表层集中，⚠ **低于本件墨量需求会截断（变浅、偏色）**；调大 ⇒ 墨往深处摊（正对视角几乎不变，过厚损失清晰度）。⚠ 它不是明度旋钮（算法仓 384_13 实测） |
| `band_uninked_fill` | 字符串，二选一 | `"transparent"` | 色带里不落彩墨的位置填什么：`"transparent"`（透明光油）或 `"white"`（白）。**只认这两个词** |
| `support_enable` | **布尔** `true` / `false` | `true` | 是否打支撑。⚠ 必须是 JSON 布尔，字符串 `"false"` 会被拒绝 |
| `output_width_px` | 正整数或 `null` | `null` | 输出 BMP 宽度（喷头数）；`null` = 按模型实际宽度 |
| `voxel_mm` | 三个正数的数组 | `[0.137, 0.035, 0.01]` | 墨滴尺寸 mm（x, y, z），z 就是层高 |
| ★`threads` | 非负整数或 `null` | `null` | 线程数；`null` / 0 = 全部硬件线程（线程数不影响结果，逐字节相同） |
| ★449 `wt_engine` | 字符串 `"legacy"` / `"unified"` | `"legacy"` | 水转印加网引擎。`legacy` = 原 HE500（**省略 = 与上一版逐字节相同**）；`unified` = HE500 / HE510 / HE520 统一的一份代码，用下面的预设 / 开关选 |
| ★449 `wt_preset` | `"he500"` / `"he510"` / `"he520"` / `"custom"` | `"he500"` | **只在 `unified` 时许给**。he500 = 不外移（= legacy 逐字节）；he510 = 法线柱全外移；he520 = 光学柔移 + 蓝噪声取整抖动；custom = 下面四个开关自己拼 |
| ★449 `wt_outward_columns` | `"normal"` / `"edt"` | `"normal"` | 只在 he510 / he520 时许给：柱沿法线重建（默认）还是用原 EDT 柱（复现 428~435 的口径） |
| ★449 `wt_outward_strength` | 数字，[0, 1] | `0.5` | 只在 he520 / custom 时许给：光学外移强度 a（0 = 不挪，1 = 挤到表面） |
| ★449 `wt_outward_attenuation_mm_inv` | 数字 > 0 | `4.0` | 只在 he520 / custom 时许给：光学衰减系数 λ（/mm），越大越往表面集中 |
| ★449 `wtu_columns` / `wtu_ray_origin` / `wtu_move` / `wtu_dither` | 字符串 | `normal` / `voxel` / `optical` / `blue_noise` | 只在 custom 时许给：柱（edt / normal）、射线起点（surface_point / voxel）、外移（none / compact / optical）、取整抖动（blue_noise / none）。**默认四个开关 = he520** |

⚠ **写了不认识的键、或者类型不对，都直接返回 2**（并说明哪个参数、收到了什么），而不是静默忽略。
★449 同理：**给了却不生效**的键也返回 2（例如 `legacy` 下给 `wt_preset`、he510 下给 `wt_outward_strength`），规则在 `params_schema.json` 的 `only_when` 里；
`legacy` 时 `slice_metadata.json` 的 `params_dict` **不回显**这 9 个键（规则 `echo_omit_when`），老调用方读到的回显与上一版相同。
★449 ⚠ 法线柱（he510 / he520 / custom normal）+ `orient_yaw_deg` 不是 90° 整数倍时，结果跟着平台数学库 sin/cos 的末位走：每个平台的库与同平台的 Python 生产线逐字节相同，但 Windows 与 Linux / macOS 之间可能差几格（实测 hemi 6 mm yaw 45：he510 两格换色、he520 落墨差 2 格）；legacy 或 yaw 为 90° 整数倍时三平台逐字节相同。

**★ 尺寸为什么有两种说法（383）**：lmst 的 `targetHeightMm` 是「摆正后的 z 高度」，算法仓界面的 `TARGET_MM` 是「最长边」。
同一个数字，**立方体上一样，扁件完全不一样**（实测 2×1×0.5 mm 长方体：z 高度 1 mm ⇒ 109 张 BMP；最长边 1 mm 只有 33 张），而且不报错。
⚠ 与 lmst 仍有一处不同：lmst 用 PCA 摆正，本 SDK 按包围盒轴摆正（斜放的模型不转）。

**参数之间的牵连（调参前必看）**：

| 你改的 | 实际还牵动了什么 |
| --- | --- |
| 任意一个 `ink_*_um`（`color_thickness_mm` 为 `null` 时） | 色带容量 = 满喷厚度之和 ⇒ **整条色带跟着变厚 / 变薄**，其它颜色的深度分布也跟着变 |
| `use_black_k=true` | 默认色带容量多加 `ink_k_um`（400+500+300+400 = 1.6 mm）；即使 `black_k_amount=0`，结果也与不开 K 不同 |
| `ink_*_um = 0` | 模型里这种颜色的需求**直接丢掉**，不会拿别的墨顶替 |
| `color_thickness_mm` 小于需求 | 一根柱装不下的墨被截断（颜色变浅、色相可能偏） |
| `output_width_px` 小于模型宽 | 跑到写 BMP 那一步才发现 ⇒ 返回 1，errorBuffer 写「输出宽度 N 小于模型本身的 M」 |

**固定、不能通过 dll 改的**：加网算法族固定为水转印（★449 起可用 `wt_engine` / `wt_preset` 在 HE500 / HE510 / HE520 之间选；`FryHalftone_GetAlgorithmName` 仍报 `water_transfer_depth`）；ICC 口径 naive；体素化走 **exact 精确路线**（★390：表面三角形与体素立方体判交 ∪ 每根竖柱的带符号绕数 ∪ 封闭空腔补实，与 Python 逐位一致）；其余参数用算法仓流程表的默认值。

## 六、支持范围

| 类别 | 支持 | 暂不支持（返回 7） |
| --- | --- | --- |
| 贴图 | GLB 内嵌 / data URI / 同目录外链的 PNG（各位深 / 调色板 / 透明）与**基线 JPEG**（灰度 / YCbCr，任意整数倍色度采样）；★391 WebP 等新格式**有 PNG/JPEG 回退就用回退** | 渐进式 / 无损 / 算术编码 / 12 位 / CMYK JPEG；没有回退的 WebP / KTX2 / AVIF / DDS |
| 几何 | 三角形 / ★391 三角条带 / 三角扇；稀疏 accessor；量化坐标（KHR_mesh_quantization）；**任意层级节点变换**（matrix 或 平移·旋转·缩放，含镜像翻面）；点 / 线图元自动忽略（不是曲面） | Draco / meshopt 压缩；GPU 实例化（EXT_mesh_gpu_instancing）；一个节点挂在多个父节点下；近刚体矩阵（偏差在 1e-13~1e-5，trimesh 会 SVD 修正）；GLB 里带 uri 的 buffer；.gltf 文本文件 |
| 方向 | ★392 `orient_mode="axis_map"` + `orient_up` / `orient_front` 两根**有符号轴**唯一定死朝向（24 种，det=+1 不镜像）；可再加 `orient_yaw_deg` 绕 z 转任意角（90° 整数倍为精确值）；省略这些键 = 老行为（按边长自动摆正） | —— |
| 颜色 | ★391 按 glTF 规范：**baseColorFactor × 贴图 × COLOR_0**（缺哪项取 1；材质没写 factor = 白）；顶点色 float / u8 / u16；TEXCOORD_n + KHR_texture_transform；wrapS/wrapT（重复 / 夹边 / 镜像）、magFilter NEAREST；高光光泽度工作流（diffuseFactor × diffuseTexture）；多块混合 | 声明「必需」而本库不认识的扩展 |
| 尺寸 | ★390 起**不按大小拒收**（体素化走 exact，与模型大小无关；42 mm 麻将牌 = 5.22 亿体素 / 2009 张 BMP，实测 20.3 s，与 Python 逐字节相同） | ——（只受内存限制：随体素数线性增长，见第一节 ②） |

坏文件（截断、越界偏移、NaN 坐标、TEXCOORD 个数与顶点数不同、像素数超过 1.79 亿）返回 1。

## 七、输出

- 输出目录里：`000000.bmp`、`000001.bmp` …（**4-bit 调色板 BMP，一张 = 一个打印层**，z 方向）
- `slice_metadata.json`：层数、第一张 / 最后一张文件名、落墨体素数、墨滴尺寸、输出宽度、SDK 版本、引擎、算法指纹，
  以及 `params_dict`（**这次生效的全部参数**）和 `size_dict`（按哪种尺寸口径、模型自带三边、缩放倍数、成品最长边、成品 z 高度）
  ⇒ 一叠切片拿出去，「哪版算法」看指纹、「什么参数、打多大」看这两项，齐了就能原样复现
- `_orient.json`：摆放朝向与输出宽度


### ★411 BMP 像素值（调色板索引）→ 材料

BMP 是 4 位调色板图，**每个像素存的是调色板索引**；调色板 16 色本身是灰度（奇数索引 = 灰阶、偶数 = 黑）
⇒ **按像素值认材料，别按颜色认**。对照表（唯一真源：`cpp/src/output/assemble.cpp` 的 `kPal`；出包测试逐项核对，改了源码这张表不跟着改就红）：

| 像素值 | 材料 | 打印机喷不喷 | 什么时候出现 |
| ---: | --- | --- | --- |
| 0 | 空（Null） | 不喷 | 模型外、不打印的位置 |
| 1 | 青（C） | 喷 | 彩墨 |
| 3 | 品红（M） | 喷 | 彩墨 |
| 5 | 黄（Y） | 喷 | 彩墨 |
| 7 | 黑（K） | 喷 | 只在 `use_black_k=true` 时出现 |
| 9 | 白（White） | 喷 | 白色材料（白底）；`band_uninked_fill="white"` 时色带里不落彩墨的格子也是它 |
| 11 | 透明（Transparent） | 喷 | 透明壳；`band_uninked_fill="transparent"`（默认）时色带里不落彩墨的格子也是它 |
| 13 | 支撑（Support） | 喷 | `support_enable=true`（默认）时 |

其余值（2、4、6、8、10、12、14、15）不会出现。⚠ **只有 0 是「什么都不打」**：白 / 透明 / 支撑都会喷。
每种材料一共多少个体素，见 `slice_metadata.json` 的 `material_count_dict`。

## 八、平台注意事项

| 平台 | 注意 |
| --- | --- |
| Windows | 无需安装 VC++ 运行库（C++ 运行库已静态链入）；C 数学库用系统自带的 `ucrtbase.dll`（Windows 10 起内置，与 numpy 用的是同一份） |
| Linux | ★409 两种包，`sdk_info.json` 的 `min_os_str` / `build_origin_str` 写明是哪种：**prebuilt**（Windows 上 zig 交叉编、glibc 2.28 基线）实测 **glibc ≥ 2.17 可加载**；**native**（Debian 13 上 gcc 14 编）要 **glibc ≥ 2.38**。libstdc++ / libgcc 已静态链入，RUNPATH 只有 `$ORIGIN` |
| macOS | arm64 / x64；最低 **macOS 13.0**（Mach-O minos 实测）；arm64 为 ad-hoc 签名、未公证：从浏览器下载后先执行 `xattr -dr com.apple.quarantine <解包目录>` |

★409 逐字节一致的实测范围：**Windows**（与 Python 生产线）＋ **Linux**（WSL2 Debian 13 / glibc 2.41：golden 13 件 + 54 个真实模型 × 两个尺寸，与 Windows 逐张 BMP 相同）。
⚠「能加载」≠「逐字节一致」：数学库（libm）是目标系统自己的，只有**跑过 golden 的系统**才算验过；macOS 需在 Mac 上跑验收包（`bash run_on_target.sh`）。

## 九、排障顺序

0. ★387 **dll 和说明是不是同一次出包的**：算一下 `bin/` 里 dll 的 sha256，与 `sdk_info.json` 的 `library_sha256_str` 比（Windows：`certutil -hashfile bin\fry_halftone.dll SHA256`）—— 不同 = 有人只换了 dll
1. `FryHalftone_GetEngine()` / `GetVersion()` —— 拿到的是 384 起的 C++ 版吗？
2. `FryHalftone_GetAlgorithmFingerprint()` —— 和发布记录里的指纹一致吗？
3. 返回 7：errorBuffer 写了哪种不支持 —— 用算法仓界面「RIP 加网出片」（Python 生产线）出这件
4. 返回 1：errorBuffer 开头就是原因；模型能在算法仓 Python 生产线跑通吗？（跑不通 = 文件本身坏了）

## 十、★409 命令行 `fry-halftone-cli`（不写代码也能出片）

三个平台的包里都带 `bin/fry-halftone-cli`（Windows 是 `.exe`），**与库放在同一个目录**。它只调上面那 11 个公开 C 接口、
动态链接**同一个** `fry_halftone` 库 ⇒ 命令行出的切片 = 库出的切片（出包时 golden 13 件全部经命令行再比一遍逐字节）。
协议与 lmst 的 `lmst-cli` 同构（云端 worker 按同一套正则解析进度），换引擎只改可执行文件名。

```
fry-halftone-cli slice <模型.glb> --out <目录> (--target-height <mm> | --target-longest-edge <mm>)
                 [--layer-height <mm>] [--no-auto-orient] [--threads <n>] [--params <参数.json>] [--json]
fry-halftone-cli run <request.json> [--json]      # 与 FryHalftone_SliceModelJsonUtf8 同一个请求 JSON
fry-halftone-cli schema                           # 库自报的参数白名单（= 包里的 params_schema.json）
fry-halftone-cli info [--json]                    # 名字 / 版本 / 算法 / 引擎 / 算法指纹
fry-halftone-cli --version                        # 恰好一行：fry-halftone-cli <版本>
```

| 约定 | 取值 |
| --- | --- |
| 输出目录 | 不存在就建；**必须是空的**（含隐藏文件），否则退出码 4 —— 防止新旧切片混在一起 |
| 进度（stderr） | 逐行 `[fry-halftone-cli] progress=N stage=...`；成功最后一行 `[fry-halftone-cli] done. slices=N` |
| `--json`（stdout） | 一行一个事件：若干 `{"type":"progress",...}` + 一个 `{"type":"result","result":{success, sliceCount, outputDirectory, metadataPath, ...}}` |
| 参数 | 开关覆盖常用项；其余写进 `--params` 的 JSON 对象（键见 `schema`）；**同一个键两处都给 = 退出码 2**（不猜谁优先） |
| 退出码 | 0 成功 ｜ 2 用法 / 参数错 ｜ 3 模型不存在 ｜ 4 输出目录问题 ｜ 11 出片失败（库返回码写在 JSON 里）｜ 12 输出不完整（张数对不上） |

例（用户的标准件：C 400 / M 500 / Y 300 µm、不用 K）：

```
echo {"ink_c_um":400,"ink_m_um":500,"ink_y_um":300,"use_black_k":false} > p.json
fry-halftone-cli slice white_球_no_c_upper_hemisphere.glb --target-height 5 --params p.json --out out_hemi
```

★ Linux 上命令行已把 libstdc++ / libgcc 静态链入、RUNPATH 只有 `$ORIGIN`（只认同目录的库）；macOS 上靠库的 install_name `@loader_path/…`。

## 十一、★411 三个平台照着敲就能跑的命令（在解包后的 `fry-halftone/` 目录里执行）

> Windows 与 Linux 两组**原样逐条实测过**（Windows 11 + MSVC + Python + .NET；WSL2 Debian 13 + cc + python3）；macOS 那组要等 Mac 验收后才能实测。
> 命令的唯一真源是打包仓 `tools/fry_integration_guide.py`（本节、`examples/c/fry_halftone_demo.c` 开头的注释、交付目录里的各平台集成说明都与它逐字一致，有测试核对）。
> `<模型.glb>` 换成你的模型路径；⚠ 可执行文件要和库在同一个目录（下面都输出到 `bin/`）。

### Windows x64（cmd（Windows 命令提示符；PowerShell 里把 copy 换成 Copy-Item））

| # | 做什么 | 命令 |
|---|---|---|
| 1 | 自检：包完整吗 / dll 能加载吗 / 版本号处处一致吗 | `python verify_delivery.py` |
| 2 | 命令行：版本 | `bin\fry-halftone-cli.exe --version` |
| 3 | 命令行：出片（最长边 12 mm） | `bin\fry-halftone-cli.exe slice <模型.glb> --target-longest-edge 12 --out out_cli` |
| 4 | C：编译示例（在「x64 Native Tools Command Prompt for VS」里执行） | `cl /nologo /utf-8 /I include examples\c\fry_halftone_demo.c lib\fry_halftone.lib /Fo:bin\ /Fe:bin\fry_halftone_demo.exe` |
| 5 | C：运行（第 3 个参数 = 成品 z 高度 mm） | `bin\fry_halftone_demo.exe <模型.glb> out_c 1.6` |
| 6 | Python：ctypes 示例（不装第三方包；自动在 ..\..\bin 找 dll） | `python examples\python\fry_halftone_demo.py <模型.glb> out_py` |
| 7 | C#：新建控制台工程 | `dotnet new console -o FryDemo --force` |
| 8 | C#：用示例替换 Program.cs | `copy /Y examples\csharp\FryHalftoneDemo.cs FryDemo\Program.cs` |
| 9 | C#：编译 | `dotnet build FryDemo -c Release -o FryDemo\out` |
| 10 | C#：把 dll 放到 exe 旁边（DllImport("fry_halftone") 就近加载） | `copy /Y bin\fry_halftone.dll FryDemo\out` |
| 11 | C#：运行 | `FryDemo\out\FryDemo.exe <模型.glb> out_cs` |

- 运行只需要 `bin\fry_halftone.dll` 一个文件；**不用装 VC++ 运行库**（C++ 运行库已静态链入，UCRT 用系统自带的，Windows 10 起内置）。
- C/C++ 隐式链接用 `lib\fry_halftone.lib`（导入库）；用 `LoadLibraryW` 动态加载则不需要它。
- 调用约定是 `__stdcall`（头文件里的 `FRY_CALL`）：C# 写 `CallingConvention.StdCall`，Python 用 `ctypes.WinDLL`。
- 中文路径：C/C++ 用宽字符版 `FryHalftone_SliceModel(const wchar_t*…)`（`wmain` 取参数），或 UTF-8 版 `…Utf8`；**不要**用本地代码页的 `char*`。
- dll 版本号还能右键看：属性 → 详细信息（文件版本 / 产品版本 = 交付版本号）。

### Linux x86-64（bash）

| # | 做什么 | 命令 |
|---|---|---|
| 1 | 自检 | `python3 verify_delivery.py` |
| 2 | 命令行：版本 | `bin/fry-halftone-cli --version` |
| 3 | 命令行：出片（最长边 12 mm） | `bin/fry-halftone-cli slice <模型.glb> --target-longest-edge 12 --out out_cli` |
| 4 | C：编译示例（输出放进 bin/：RUNPATH=$ORIGIN 只认同目录的 .so） | `cc examples/c/fry_halftone_demo.c -Iinclude -Lbin -lfry_halftone -Wl,-rpath,'$ORIGIN' -o bin/fry_halftone_demo` |
| 5 | C：运行（第 3 个参数 = 成品 z 高度 mm） | `bin/fry_halftone_demo <模型.glb> out_c 1.6` |
| 6 | Python：ctypes 示例 | `python3 examples/python/fry_halftone_demo.py <模型.glb> out_py` |

- 运行只需要 `bin/libfry_halftone.so`；libstdc++ / libgcc 已静态链入，实测只要求 **glibc ≥ 2.17**（CentOS 7 起）。
- 你自己的程序链接它：`-L<包>/bin -lfry_halftone`，运行时让加载器找得到它 —— 三选一：程序与 .so 同目录 + `-Wl,-rpath,'$ORIGIN'`；或 `-Wl,-rpath,<.so 所在目录>`；或 `LD_LIBRARY_PATH=<.so 所在目录>`。
- Python 用 `ctypes.CDLL`（不是 WinDLL）；C# 在 Linux 上 `DllImport("fry_halftone")` 会自动找 `libfry_halftone.so`（本机未实测 .NET on Linux）。
- 逐字节一致在 glibc 2.41（WSL2 Debian 13）上实测；你们服务器的 glibc 不同时，建议用验收包在服务器上跑一次（见 macOS 目录里的验收包，它也带 Linux 目标）。

### macOS（arm64 / x64，Mac 验收通过之后）（zsh / bash）

| # | 做什么 | 命令 |
|---|---|---|
| 1 | 解除下载隔离（从浏览器 / 聊天工具下载的才需要） | `xattr -dr com.apple.quarantine .` |
| 2 | 自检 | `python3 verify_delivery.py` |
| 3 | 命令行：版本 | `bin/fry-halftone-cli --version` |
| 4 | 命令行：出片（最长边 12 mm） | `bin/fry-halftone-cli slice <模型.glb> --target-longest-edge 12 --out out_cli` |
| 5 | C：编译示例（输出放进 bin/：库的 install_name 是 @loader_path/，只认同目录） | `cc examples/c/fry_halftone_demo.c -Iinclude -Lbin -lfry_halftone -o bin/fry_halftone_demo` |
| 6 | C：运行 | `bin/fry_halftone_demo <模型.glb> out_c 1.6` |
| 7 | Python：ctypes 示例 | `python3 examples/python/fry_halftone_demo.py <模型.glb> out_py` |

- **先在 Mac 上跑验收包**（`03_macOS_需在Mac上验收/macOS验收步骤.md`），通过后 `results/<目标>/` 里的 `.tar.zst` 才是 macOS 包；下面的命令在它解开的 `fry-halftone/` 里执行。
- 库的 install_name 是 `@loader_path/libfry_halftone.dylib` ⇒ **你的可执行文件（或加载它的那个库）要与 dylib 在同一个目录**；放别处就用 `install_name_tool -change` 改。
- arm64 版是 ad-hoc 签名、未公证：只给同事内部用没问题；要发给外部客户需要公司的 Developer ID 签名 + 公证。
- Python 用 `ctypes.CDLL`；C# 在 macOS 上 `DllImport("fry_halftone")` 会找 `libfry_halftone.dylib`（未实测）。
- 本页命令还没在 Mac 上逐条跑过（Windows 上没有合法的 macOS 环境）；Mac 验收时 `run_on_target.sh` 会真跑预编的纯 C 示例、命令行契约（tests/test_cli_contract.py 全部用例）与交付清单。

## 十二、★411 版本号

| 项 | 规则 |
|---|---|
| 格式 | 交付版一律 `X.Y.Z`（纯数字三段）；**0.1.0 起，每次交付递增** |
| ★412 升哪一位（出包时自动判定） | 按 [SemVer](https://semver.org/lang/zh-CN/)，与上一次交付比「对外接口面」（C 接口的函数签名 / 参数名与类型 / 命令行的子命令与开关）和算法指纹：**删了或改了**任何一项 = 不兼容 → 主版本；**加了**一项、或**算法指纹变了**（同样的输入会出不同的切片，你那边要重新验证）→ 次版本；只有实现、文档、打包变了 → 修订号。⚠ **0.x 期间**（接口还没承诺稳定）不兼容的改动只升次版本；**1.0.0 由人决定**（Mac 验收通过、接口承诺稳定之后） |
| ★412 你据此能判断什么 | 只升了**修订号** ⇒ 同样的输入出的 BMP 逐字节不变，可以直接换；升了**次版本** ⇒ 先看 `CHANGELOG.md` 这一版的节和交付说明第一节「升了哪一位（为什么）」，输出可能变了，建议重新验证；0.x 期间次版本也可能含不兼容改动（CHANGELOG 会写明删了 / 改了什么） |
| 在哪里看 | 包名 `fry-halftone-<版本>-<平台>`、`VERSION`、`sdk_info.json`、`FryHalftone_GetVersion()`、`fry-halftone-cli --version`、Windows dll 右键属性 → 详细信息 —— 出包时逐处核对必须一致 |
| 改了什么 | `CHANGELOG.md` 按版本号分节：`## X.Y.Z（日期）` 下面就是这一版相对上一版的变更 |
| `0.0.0-dev` | 本地调试构建的占位值（没指定版本号时）：**这种包不交付**，交付工具会拒绝 |
| 同一版本号 | 只对应一批字节；版本号只增不减，打包仓 `delivery_versions.json` 记着每次交付的版本、日期、两个指纹与各包 sha256 |
