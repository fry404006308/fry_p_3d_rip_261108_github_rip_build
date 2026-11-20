# CHANGELOG

## 未发布

## 0.4.0（2026-11-20）

- 新增 orient_mode=original；采用 GLB 场景原轴，要求 yaw 为零。兼容旧版 axis_map/+z/+y。


（下一次交付的变更记在这里：交付时会自动挪到新版本号下面）

## 0.2.0（2026-10-27）

### ★449 水转印统一引擎（HE500 / HE510 / HE520 一份代码）进 SDK；新增一份 dll、原先的不动（legacy 输出一个字节没变）
- **新参数 9 个**（库自报参数说明 18 → 27 个；全部可省略，省略 = 与上一版逐字节相同）：
  `wt_engine`（legacy 默认 / unified）、`wt_preset`（he500 / he510 / he520 / custom）、`wt_outward_columns`、`wt_outward_strength`、
  `wt_outward_attenuation_mm_inv`、`wtu_columns`、`wtu_ray_origin`、`wtu_move`、`wtu_dither`。
  「给了却不生效」的组合当场报错（如 legacy 下给 `wt_preset`），规则写在参数说明的 `only_when` 里；
  legacy 时 `slice_metadata.json` 不回显这 9 个键（参数说明新增 `echo_omit_when` 规则，老调用方读到的回显与上一版相同）。
- **golden 13 → 23 件**：新增 10 件统一引擎（三个预设、HE520 强度 / λ、柱口径 edt、自定义两组、加网方向 yaw 90、大件 box_l12），
  标准答案由算法仓 **Python 生产线**出（`water_transfer_unified[_heXXX]`，与旧 HE500/510/520 逐体素相同），不是拿 dll 自己的输出当答案。
- C 接口用例 ⑩：四个预设逐个「省略默认值 = 显式给默认值」（参数回显 + 切片 md5 都相同），he500 预设 = legacy 逐字节，三个预设互不相同；
  ⑪ 参数探针按 `only_when` 规则自动补齐前提（164 个探针 0 处不符）。
- **变体交付**：`fry_deliver.py --variant <关键词>` 交付到 `_bin/<平台>_<YYMMDD_HHMM>_<关键词>/`，原先的 `_bin/<平台>/` 不碰（前后核对 sha256）；
  `fry_release.py --deliver-variant` 出齐三平台后顺手交付（时间戳与全交付目录相同）；`--bundle-topic` 可写 `{version}`。
- **双击出包**：`tools/fry_oneclick_release.bat`（纯 ASCII）→ `tools/fry_oneclick_release.py`（中文默认值、日志与结论写进 `out/oneclick/`）。
- `sdk_info.json` 新增 `wt_engine_choices_dict`（库认哪些引擎 / 预设，取自库自报的参数说明）。
- ⚠ **已知的平台差异（实测，不是 bug）**：法线柱（he510 / he520 / custom 的 normal 柱）+ `orient_yaw_deg` 不是 90° 整数倍时，
  结果跟着平台数学库的 `sin` / `cos` 末位走（sin 45°：Windows UCRT `…3bcd`，Linux glibc `…3bcc`）⇒ **每个平台的库 = 同平台的 Python 生产线**
  （hemi 6 mm yaw45 he510：Windows dll = Windows Python `85b405a9`，Linux dll = Linux Python `45f1da6d`，落墨数同为 110096），但两平台之间差几格。
  legacy 与 yaw 为 90° 整数倍时三平台逐字节相同；跨平台 golden 因此用 yaw 90。
- 出包：仓库搬家后 `out/cmake` 的 CMake 缓存记着旧路径 ⇒ 自动清掉重配（以前直接报错退出）。

## 0.1.3（2026-09-29）

（本版没有功能变更：只是重新打包）

## 0.1.2（2026-09-24）

### ★412 版本号自动定升哪一位；一键出包自带全部证据；示例补上黑墨与白底（算法输出一个字节没变）
- **升哪一位自动判定**（`tools/fry_surface.py`，SemVer）：出包前从源码解析「对外接口面」（C 接口签名 / 参数名与类型 / 命令行子命令与开关），
  与上一次交付记录比：删了或改了 → 主版本（**0.x 期间降为次版本**，1.0.0 由人决定）；加了、或算法指纹变了 → 次版本；只有实现 / 文档 / 打包 → 修订号。
  出包后再用产物量一遍（库自报参数表 / 真跑 `--help` / 库导出表），与源码解析不一致就报错；接口面记进 `delivery_versions.json`，下一版据此比。
  上一版的记录里没有接口面（0.1.0 / 0.1.1）时：两个指纹都没变 ⇒ 接口面必然没变（推断）；变了 ⇒ 要人给 `--bump`。
- **CHANGELOG 残留节改号**：上次出包分了节却没交付成（中途失败），这次判定要升更高的位 ⇒ 那一节改成这次的号（内容原样），已交付的节永远不动。
- **一键出包自带全部证据**（`fry_release.py --bundle`）：集成命令实测 → ★跨平台一致性复核（`tools/fry_parity_check.py`，真实模型逐张逐字节）
  → ★验收包在 WSL 真 Linux 里完整跑一遍（`tools/fry_kit_check.py`，含反向篡改）→ 打交付目录 → ★站在同事角度复验整包
  （`tools/fry_bundle_check.py`：解开 zip → 校验 → 命令行不 chmod 直接跑 → 每个示例照说明重出、逐字节比）；**复验不过 ⇒ 目录挪进 `_作废/`、不记版本号**。
- **示例 1 个 → 3 个**：默认参数 ｜ ★黑墨 K（像素值 7）｜ ★白底填充（像素值 9）⇒ 调色板 8 个值在示例里全部出现、逐值核对像素数 = 库自报的材料计数；
  带参数的示例用 `run request.json`（与 C 接口 `FryHalftone_SliceModelJsonUtf8` 同一份请求）。

## 0.1.1（2026-09-23）

### ★411 说明补上「BMP 像素值 → 材料」对照表（算法输出一个字节没变）
- BMP 是 4 位调色板图，而调色板 16 色本身是**灰度** ⇒ 只能按像素值认材料；以前的说明只写了「4-bit 调色板 BMP」，没写哪个值是什么材料。
  `SDK_使用说明.md` 第七节 / `SDK_README_EN.md` 新增对照表：0 空 / 1 青 / 3 品红 / 5 黄 / 7 黑 / 9 白 / 11 透明 / 13 支撑
  （其余值不会出现；**只有 0 不喷**，白 / 透明 / 支撑都会喷）。
- 出包测试核对这张表 = C++ 源码（`kPal`）；全交付目录再拿示例输出逐像素核对：按表数出的各材料像素数 = 库在 `slice_metadata.json`
  的 `material_count_dict` 里自报的体素数（两个互相独立的来源，写错一行就对不上）。
- 全交付目录：说明里给出这张表和示例里每种像素值的实际像素数；版本记录标出已作废的目录；接口表里「见第X节」改成指向
  `SDK_使用说明.md`；`06_验收证据/` 逐文件写明是什么；可附「验收包在真 Linux 上跑通」的报告（过闸后写进说明与 Mac 验收步骤）。

## 0.1.0（2026-09-23）

### ★411 交付版本号从 0.1.0 起递增；全交付目录「信息尽量多」；修 SHA256SUMS 里过期的一行（算法输出一个字节没变）
- **版本号**：交付版一律 `X.Y.Z`，**0.1.0 起每次交付递增**（`tools/fry_delivery_version.py`；历次记录在进 git 的
  `delivery_versions.json`：版本 / 日期 / 目录 / 两个指纹 / 三个包的 sha256）。`0.0.0-dev` = 本地调试构建，全交付目录拒收；
  版本号只增不减、同一个版本号只对应一批字节；出包前把 CHANGELOG「未发布」挪到「## X.Y.Z（日期）」下（三个平台包里是同一份）。
  `fry_release.py --bundle` 自动取下一个版本号（`--bump minor|major` 可改）；`--tag` 正式版也必须大于已交付的最大版本号。
- **全交付目录 v2**：每个平台的包**展开**放（`include/`、`lib/`、`examples/`、全部说明、自检脚本直接可见，不用先解压）；
  各平台《集成说明》（C / Python / C# / 命令行，**每条命令都原样真跑过**：Windows = MSVC + Python + .NET，Linux = gcc + python3，
  逐条结果随交付给出；macOS 等 Mac 验收）；接口 / 参数 / 命令行速查（签名取头文件、参数表取库自报说明、`--help` 取真跑输出）；
  `05_示例输出/`（交付的命令行真跑一次：输入模型、命令、全部 BMP、元数据、进度与 JSON 事件，**与标准答案逐字节相同**才放行）；
  `07_版本记录/`（历次交付 + 本版变更原文）。整包 zip 记 Unix 权限（Linux / Mac 上解开，展开的命令行直接能跑）。
- **修 SHA256SUMS**：393 起每个包里 `SHA256SUMS.txt` 记的 `delivery_manifest.json` 那一行都是过期的（封包时写完 SUMS 又重写了一次清单）
  ⇒ 对方跑 `sha256sum -c SHA256SUMS.txt` 会报 1 个 FAILED；而 `verify_delivery.py` 与出包冒烟只按清单核对 ⇒ 一直绿。
  现在封包顺序只有一种（`fry_delivery_contract.seal`：先清单、**最后** SUMS；清单不再记 SUMS 的 sha256，否则互相覆盖成环），
  并且出包冒烟与 `verify_delivery.py` 都加了第二把尺子「按 SHA256SUMS 逐行核对」。
- **修示例里的编译命令**：照抄会失败 —— Windows 把导入库写成 `bin\fry_halftone.lib`（实际在 `lib\`，LNK1181）；
  Linux / macOS 没写输出位置，可执行文件落在包根，而 RUNPATH=`$ORIGIN` / install_name=`@loader_path/` 只认同目录 ⇒ 一运行就找不到库。
  改成输出到 `bin/`（与库同目录）的命令；命令只写一份（`tools/fry_integration_guide.py`），说明与实测都从它取。
- `SDK_使用说明.md` 新增第十一节（三个平台照着敲就能跑的命令）、第十二节（版本号规则）；`GetVersion()` 的说明改成交付版 `X.Y.Z`。

### ★410 全交付目录：三个平台 + 命令行 + 说明 + Mac 验收步骤一次放齐，每次一个新子目录（算法输出一个字节没变）
- **`tools/fry_delivery_bundle.py`**：过闸后放进 `out/全交付目录/_YYMMDD_HHMM_<主题>/`，另打一个同名 zip（中文文件名 UTF-8，
  Mac / Linux 解压不乱码）；目录里 `00_先读我_交付说明.md`、`01_Windows_x64/`、`02_Linux_x64/`、`03_macOS_需在Mac上验收/`
  （验收包 + 《macOS验收步骤.md》：怎么验、**怎样才算成功**、发回什么、没成功怎么办）、`04_对接文档/`、`05_验收证据/`、
  `bundle_manifest.json`、`SHA256SUMS.txt`；`out/全交付目录/_交付记录.md` 每次追加一行。同一分钟再跑不覆盖（`_2`）。
- **闸**：同一版本、同一算法指纹、**同一外壳指纹**、Windows / Linux 冒烟都过（含命令行契约）、验收包 sha256 = kit_info、
  验收包带两个 macOS 目标且自称「未验证」、参数说明各平台逐字节相同；附一致性实验时它必须「全一致」且**跑的就是交付的那份库**
  （实验记下的库 sha256 = 交付包 sdk_info 里的；409 那种只记版本字符串的旧格式拒收）。装完按 SHA256SUMS 复查、zip 重开复查。
  说明里「应该看到的数」（golden 几件、交付清单几项、导出几个接口）从 `tests/data/golden.json` / 交付合同 / build_report 现算。
- **外壳指纹 `wrapper_sha256_str`**（`fry_build.wrapper_fingerprint`）：算法指纹只管 `cpp/`；命令行 / dll 外壳 / 导出表 / 探针 /
  示例改了而某个平台没重出，算法指纹照样相等、不报错 ⇒ 把 `cpp/` 之外会编进二进制的输入也算一个树 sha256（CRLF→LF）。
  写进 build_report、prebuilt_info、kit_info；`--prebuilt-dir` 时核对「预编件记录 = 当前源码」（验收包里没有源码就信记录）；
  `tests/test_build_helpers.py` 的闭包闸保证 CMakeLists.txt 引用的每个输入都在清单里。
- **`fry_release.py --bundle`**：一键出齐（没点 Linux 就补 prebuilt；交叉编四个目标、验收包带全部目标 ⇒ 同一个验收包也能拿到 Linux 服务器上验）
  再打全交付目录；交付目录只放一个 Linux 包（prebuilt：glibc ≥ 2.17；native 要 ≥ 2.38，只当对照）。
- **验收脚本**：`results/<目标>/` 多一份 `SHA256SUMS.txt`（`shasum -a 256 -c` / `sha256sum -c`），收回时能核对包在路上没坏。

### ★409 三平台：Linux 在本机 WSL 真验收出包；macOS 出「验收包」一条命令验；三个包都带命令行（算法输出一个字节没变）
- **命令行 `bin/fry-halftone-cli`**（三平台包都带）：只调公开 C 接口、动态链同一个库；协议与 lmst-cli 同构
  （`slice … --target-height | --target-longest-edge`、stderr `progress=N` / `done. slices=N`、`--json` 事件、退出码 0/2/3/4/11/12）。
  出包冒烟新增「命令行契约」34 项：golden 13 件全部经命令行再比一遍逐字节。
- **Linux 真出包**（`tools/fry_build_linux_native.py`）：在 WSL2 / Docker 的真 Linux 里跑同一套冒烟 ⇒ 可交付。
  `--mode native`（gcc 原生编）｜`--mode prebuilt`（Windows 上 zig 交叉编的二进制，拿到真 Linux 里验收：`fry_build.py --prebuilt-dir`）。
  修掉四个第一次真跑才暴露的问题：原地构建与 Windows 共用 `out/cmake`、`||` / `&&` 优先级让 apt 永远不带 sudo、
  `wsl --list` 是 UTF-16、Docker 同样原地构建。
- **最低系统改成实测**（以前抄配置写的「glibc 2.28+ / macOS 12+」都不对）：ELF 取引用的最高 `GLIBC_x.y`，Mach-O 取 `minos`。
  实测：gcc 原生版要 glibc 2.38、zig 版 2.17、macOS 13.0。
- **macOS 验收包**（`tools/fry_target_kit.py`）：预编二进制 + 判据 + `run_on_target.sh`，**不带算法源码**；
  Mac 上一条命令 = 指纹闸 → 装配 → 冒烟（golden 逐字节）→ 出包；Apple Silicon 上 `--also-x64` 用 Rosetta 2 顺带验 Intel 版。
  没有源码时 `fry_build.py` 把「载荷闸」换成「指纹闸」；冒烟新增「库自报指纹 = 锁文件」（防过期的预编件与说明各说各话）。
- 交叉编译的三个**静默**坑（都 rc=0、零警告，靠新加的静态闸抓到）：
  ① zig 的 Mach-O 链接器忽略 `-exported_symbols_list` ⇒ 导出 25 个（多 12 个 operator new/delete）⇒ 库里自带隐藏的 new/delete
     （`src/dll/fry_hidden_new_delete.cpp`，只在 zig 编 macOS 时编入）；
  ② CMake 的 `BUILD_RPATH` 是**追加**不是替换 ⇒ Linux 命令行 / 示例的 RUNPATH 带着 `C:/…/cmake`、macOS 带 LC_RPATH ⇒
     改 `BUILD_WITH_INSTALL_RPATH` / `SKIP_BUILD_RPATH`；
  ③ zig 预编 libc++ 带调试信息：`.so` 9.18 MB 里 8.01 MB 是 `.debug_*` ⇒ **链接时**剥（验收的那一份 = 发出去的那一份），包 4.05 → 0.60 MB。
- 交付清单：Windows 文件属性那一处版本号只在 Windows 上查（Linux / macOS 没有这种资源，以前在 Linux 上必红）；命令行进 integration 等级。
- `fry_release.py`：`--linux native|prebuilt|both`、`--mac-kit`；交叉编译并成一次；点名要的产物失败 = 整次失败（探针除外）。
- 命令行回显的数字改成「能精确还原的最短写法」（15→16→17 位有效数字依次试）：`16.851` 以前写成 `16.850999999999999`（值没错、人看着别扭）；切片逐字节不变（标准件 852 张 md5 与改前相同）。
- `fry_platform_probe.py`：WSL 判定复用 `fry_build_linux_native.wsl_distro()`（`WSL_UTF8=1` + 返回码 + 要有发行版名）。以前只看文字里有没有 "not installed"，没有发行版时（"has no installed distributions"）被**误判成可用**；加了两向测试（假的 wsl 程序）。

### ★393 交付完整性：给出去的包现在带齐「dll 必须的东西」，并且是**一道硬闸**（算法输出一个字节没变）
- 背景：这个库不只自己的软件用，同事和公司别的软件也要集成。只给一个 dll，对方一定回来要：
  头文件、导入库、版本号、参数说明、示例、许可、校验值…… ⇒ 「交付完整」必须是可执行的判据，不是口头习惯。
- **交付清单**（唯一真源 `tools/fry_delivery_contract.py`）分三个等级（包含关系）：
  `runtime`（把 dll 换上去跑起来，6 项）⊂ `integration`（别的软件链接/调用，12 项）⊂ `full`（完整交付，**17 项**）。
  每一项都写明「为什么必需」，随包固化成机读的 `delivery_manifest.json`（含每个文件的 sha256）。
- **新增交付物**：`examples/csharp/`（C# P/Invoke）、`examples/python/`（ctypes）、`LICENSE.txt`（许可与第三方声明：
  本库不链接任何第三方开源库）、`CHANGELOG.md`、`SHA256SUMS.txt`、`delivery_manifest.json`、
  `verify_delivery.py`（★ **对方自己就能验**：包完整吗 / dll 能加载吗 / 版本号一致吗）。
- ★ **Windows 文件属性（VERSIONINFO）**：以前右键 dll →「属性 → 详细信息」是**空白**的。现在有版本、产品、公司、
  版权、原始文件名，以及算法名与算法指纹。版本号来源仍只有一个（CI 注入的 `FRY_SDK_VERSION`）。
- ★ **出包硬闸**（`fry_build.py` 冒烟第 6 步）：清单 17 项齐全 + **版本号五处一致**
  （`VERSION` ｜ `sdk_info.json` ｜ `FryHalftone_GetVersion()` ｜ **文件属性** ｜ 包名）+ 文件属性逐字 == 构建时打算写的。
  不过就不出包、不交付、不打给业务的 zip。
- ⚠⚠ 踩到的坑：`rc.exe` 默认按系统 ANSI 代码页读 `.rc`，而我们的资源脚本是 UTF-8 ⇒ 中文属性全是**乱码**，
  而「版本号一致」那道闸照样绿。⇒ 加 `/c65001`，并把**每一栏**与构建输入逐字比对（有资源 ≠ 资源是对的）。
- `sdk_info.json` 扩了一节「集成信息」：ABI / 调用约定、编译器、运行库（**不需要装 VC++ Redistributable**）、
  最低系统、线程模型、内存量级、是否联网写注册表、导出符号表、返回码表、文件属性、自检命令。
- 给业务的 zip 从「dll + 三个说明」改成**完整包**（17 项 + 一份「只想换 dll」的说明）；打完解开再复查一次。
- 交付回算法仓时：装机目录也带 `VERSION` / `verify_delivery.py` / `delivery_manifest.json`（runtime 等级），
  交付完当场按清单查一遍；⚠ dll 被占用（界面开着）时**改名让路**而不是失败，并提示「那个软件重启后才用上新 dll」。

### ★392 加网方向：模型按什么方向摆进打印机，现在可以唯一指定（C 接口不变；新增 4 个参数，默认行为逐字节不变）
- 背景：392 之前只有「按边长自动摆正」（最长→x、次长→y、最短→z）。它只定**哪条边配哪根轴**，不定正负 ——
  同一个置换下不镜像的符号组合还有 4 种（头朝上/朝下、正面朝前/朝后），**定不唯一**；而且没法指定。
- 新参数（都可省略，省略 = 老行为）：
  · `orient_mode`：`"auto_extent"`（默认，老行为）/ `"axis_map"`（指定方向）
  · `orient_up`：模型的哪条**有符号轴**朝上 ⇒ voxel +z（逐层方向；BMP 张数 = 这条边 ÷ 层厚）
  · `orient_front`：模型的哪条有符号轴朝正面 ⇒ voxel +y（运行方向）；voxel +x（喷孔方向）= front × up **自动定出来**
  · `orient_yaw_deg`：定好 up/front 之后再绕 voxel z 转的角度（90° 的整数倍是精确值）
  · 轴名两套可混用：`+x -x +y -y +z -z`（模型自身轴）／`+long -long +mid -mid +short -short`（包围盒边长）
- ★ **唯一性**：6（up）× 4（与 up 垂直的 front）= **24 种**，正好是立方体的 24 个旋转；矩阵 det = +1 ⇒ 纯旋转，绝不镜像。
  实测：老的 `auto_extent` 等价于 `axis_map` 的 `up="+short", front="+mid"` —— 它一直就是这 24 种里的 1 种，只是以前改不了。
- ⚠ `target_height_mm`（z 高度口径）现在按**摆正后真正落到 z 的那条边**算（默认朝向下与以前数值完全相同）。
- ⚠⚠ 一条踩出来的坑：**只有置换矩阵时，`np.dot` 与手写三项累加才恒等**。yaw 让一行出现两个非零项后，
  两种求和顺序会差 1 ulp，经体素化放大 ⇒ 实测 402 / 433 张 BMP 不同。⇒ Python 与 C++ 现在都用**同一串显式乘加**。
- 验收：golden **12 / 12 逐字节一致**（10 件原有 + 新增「最长边立起来」与「斜放 45°」两件，让三平台 CI 也跑到新代码）；
  算法仓 Python ↔ C++ 对账 **14 / 14**（2 个模型 × 7 种朝向，含 yaw 45°）；算法仓位级基线 **与冻结基线完全一致**（默认没动）。

### ★391 取色按 glTF 规范重写：顶点色模型不再「加网后没有颜色」（C 接口不变；算法输出**有意**改变，golden 重出 8 件 + 新增 2 件）
- ⚠⚠ **根因**：取色只看 trimesh 的 visual —— 图元**有材质**时 trimesh 把 `COLOR_0` 藏进 `vertex_attributes`（`Scene.dump()` 复制时连这里都丢），
  visual 报「贴图」却没有图 ⇒ 按白色 `baseColorFactor` 取色 ⇒ LMST 两件测试模型（`sphere_flower` / `cmy 全家桶`：白 factor × 顶点色）
  送印 BMP **一格彩墨都没有**（修后 472 万 / 234 万格）。384 的 C++ 版逐位复刻了这一行为（当时记作「COLOR_0 被材质覆盖」）
- ★ 颜色改为从原始 glTF 解析（算法仓 `fry_kernel/gltf_color.py` 为正本，C++ `src/io/glb.cpp` + `src/color/sampler.cpp` 逐项同式）：
  `clip(baseColorFactor × 贴图(uv) × COLOR_0, 0, 1)`，缺哪项取 1；材质没写 factor = 白（trimesh 给 102 灰）；factor 取原值（不再先转 8 位、不再「>1 就 ÷255」）；
  顶点色 u16 ÷65535（trimesh 按 256 取模）；`TEXCOORD_n` + `KHR_texture_transform`；贴图按**像素中心**取样（x = u·W − 0.5）+ `wrapS/wrapT` + `magFilter` NEAREST；
  高光光泽度工作流取 `diffuseFactor × diffuseTexture`
- 口径：三项按存储值直接相乘（「显示域」，与算法仓 GT 渲染器默认 FLAT 着色、LMST / Datakit 模型里存的 sRGB 取值一致）
- ★ 新支持：三角条带 / 三角扇、稀疏 accessor、量化坐标与 uv（`KHR_mesh_quantization`）、data URI / 同目录外链贴图、点线图元自动忽略；
  有 PNG/JPEG 回退的 WebP 等用回退
- 返回 7（新增）：Draco / meshopt 压缩、GPU 实例化、声明必需而不认识的扩展、GLB 里带 uri 的 buffer、`.gltf` 文本文件、没有回退的 WebP / KTX2 / AVIF / DDS
- 坏文件防护：accessor **先核边界再分配**（391 fuzz 第一轮撞出：坏 count 让一次载入吃 8.9 GB）；fuzz 三轮 8.32 万次坏输入（含完整作业 1.32 万次）无崩溃、无卡死
- 载荷指纹 `a542ed3f…` → **`5d6b574b2b093d7f…`**（391 中间版 `ebde3e38…` 与它只差 `src/io/glb.h` 一行注释：不支持清单里「第 2 个及以后的 buffer」改成「带 uri 的 buffer」，与代码一致）；golden 10 件（无材质顶点色的 3 件 md5 一个字节没变 —— 那条路本来就对；贴图件因像素中心口径有亚像素级变化）
- 验收：算法仓 24 件上色方式测试件 + 全家桶 + 5 件真实模型 Python ↔ C++ **逐字节一致**（22 个中间数组 + 全部 BMP；只有 WebP 的那件 C++ 明确返回 7）；
  用户测试目录全部 **52 件**（原 27 件 + 391 新增 25 件）走界面同一条 Python 生产线（含六视图仿真 / 仿真 GLB / 四条约束）与本 dll：51 件送印 BMP 逐字节一致、只有 WebP 的那件返回 7，约束 52 / 52 全过
- golden `gltf_modes_mix`（上色方式全家桶小号）：初版测试件里 m20 的第二个实例转 45°，一是竖棱插进第一个实例 0.085 mm，二是 45° 斜平面让算法仓的仿真 GLB 出退化三角形（与取色无关）⇒ 改成转 90° 后重出（`b58aae963cc4`）；其余 9 件重跑后答案一个字节没变

### ★390 体素化换成 exact 精确路线：不再按模型大小拒收（C 接口不变；算法输出**有意**改变，golden 8 件重出）
- ⚠⚠ 原来「细分规模指标 > 3000 万 ⇒ Python 改走 ray（Embree + libigl，C++ 复刻不了）⇒ C++ 返回 7」这条分岔**整条拆掉**：
  体素化默认改为 **exact**（表面三角形与体素立方体分离轴判交 13 轴 ∪ 每根 (x,y) 柱沿 z 的带符号绕数 ∪ 封闭空腔从边界灌水补实），
  只有四则运算与比较 ⇒ C++ 与 Python **逐位一致**，与模型大小无关
- 实测：42 mm 麻将牌「發 / 萬」（指标 338.60M / 335.90M、5.22 亿体素）以前返回 7，现在 **20.3 s / 20.4 s** 出 2009 张 BMP，与 Python 逐字节相同
- 表面判交加「主轴平面带」（每个 (u,v) 只测平面够得着的 ≤ 4 格）：斜的大三角形 454.64M 对 → 14.45M 对，命中集合一个不差；多线程（`run_threads_indexed`）
- 返回码 7 **保留**给其它不支持的特性（渐进式 JPEG、实例化节点、近刚体矩阵等），**不再因为模型大**而出现
- 载荷指纹 `46551b77…` → **`a542ed3f26d21e5c…`**；golden 8 件重出（3 件轴对齐长方体 md5 一个字节没变，其余多认约 0.05% 的边界格，属有意改变）
- 验收：算法仓 27 件真实模型 dll 与 Python 的 BMP **逐字节一致 27 / 27**；出包冒烟 golden **8 / 8**（算法仓 docs 390）

### ★388 为搬到公司内网 GitLab 做准备（产物与 C 接口不变）
- `ci/internal.gitlab-ci.yml` 对齐同事模板 v1.21.2：Linux 出包 job 改用模板的打包池 `$LINUX_PACKAGE_RUNNER_TAG`（OPS-0029 按信任边界分池，
  测试池可迁 AWS、打包池固定留办公室）；文件头写清三台 runner 各要装什么（**三平台都要 git + node ≥ 18**，Windows 另要 PowerShell 7）、
  mac runner 只接受受保护 ref、自动提交程序每次推送都会起一条只算版本号的流水线
- CI 两份变体都住 `ci/`（新增 `ci/standalone.gitlab-ci.yml`），仓根 `.gitlab-ci.yml` 必须与其中之一逐字节相同（`tests/test_ci_yaml.py`）：
  切换 = 整份复制 internal，回退 = 整份复制 standalone；切换后测试照样全绿（以前独立模式的测试会跟着红）
- `tools/fry_release.py`：仓根 CI 是内网模式时，本地 `--tag` 在同步 / 出包之前就拒绝（发版改在 GitLab 点 `release:tag`：版本号由模板算、先查整条流水线全绿）；
  `--force-manual-tag` 是 OPS-0027 留的逃生口
- 新增 `tools/fry_intranet_preflight.py`（只读体检）与 `tests/test_intranet_preflight.py`；CI `check` 任务加跑它
- `ci/internal.gitlab-ci.yml` 新增 `test:unit`（独立模式 `check` 任务的内网版：同一镜像、同一组测试文件 + 载荷闸；继承模板底座 ⇒ 推送时等扳机、tag / MR 自动跑，
  红了发版绿灯闸会拦）—— 以前切到内网后纯函数闸只剩本机 pytest；`tests/test_ci_yaml.py` 与体检脚本都核对它在、测试文件集合与 `check` 相同
- 本地彩排（算法仓 388 文档包）：整段历史推内网约 145 MB vs 干净快照约 1.6 MB（约 92 倍，文件树哈希相同）；模板 `manifest.mjs` / `publish-package.mjs`
  对 `out/` 前缀按文件名记账上传；带 `-p<流水线号>.g<sha>` 的版本号穿过 CMake 注入与冒烟

### ★387 包里带参数说明 + dll 的 sha256（ABI 不变；算法输出逐字节不变，golden 8 / 8）
- `FryHalftone_GetParamSchemaJson()` 升到 `fry-halftone-params/3`：每个参数新增 `default`（省略该键时库真正用的值）、`nullable`，
  以及按类型的 `min` / `min_exclusive` / `max` / `integer` / `choices` / `length`；顶层新增 `one_of_required`（尺寸二选一）、`requires`（开 K 墨必须给 K 厚度）。
  v2 的 `name` / `kind` / `description` 不变（只加字段）。默认值取自 C++ 同一组结构体字段（与结果回显 `params` 同一个写法），不另写一份
- SDK 包新增 `params_schema.json`（出包时原样导出库自报）与 `SDK_参数说明.md`（由它逐行生成，不要手改）
- `sdk_info.json` 新增 `library_sha256_str` / `import_library_sha256_str` / `params_schema_*` / `param_count_int`
  （以前只有交付进算法仓时才写 sha256 ⇒ 直接用 SDK 包时「dll 与说明是否配套」核对不了）
- 出包冒烟新增：包内说明闸（sha256 / 说明文件 = 库现场自报 / 人读版每参数一行）；C 接口用例 ⑩ 默认值真跑（只给尺寸 vs 默认值全写出 ⇒ md5 相同；
  写错一个默认值 ⇒ md5 变）、⑪ 取值范围真跑（按说明生成 75 个压线值逐个喂库，收拒与说明一致）
- `fry_deliver.py` 连参数说明一起交付，并先核对包里 dll 的 sha256；`fry_release.py` 结果里带 `library_sha256_str` / `param_count_int` / `params_doc_path_str`

### ★384 算法改为 C++（接口形状不变；返回码 3 / 4 不再出现；新增 7）
- ⚠⚠ **引擎换代**：算法从「PyInstaller 冻结的 Python 子进程」改为**进程内 C++**（源码 = 算法仓 `fry_project_components_cpp/`，
  打包仓副本 `cpp/` + `cpp.lock.json`）。与 Python 生产线**逐字节一致**：算法仓 21 个模型 × 23 项中间结果全量对账 + 本仓 golden 8 件
- 包体积 95.5 MB → **0.35 MB**；出包 135~150 s → **19 s**；半球 16.851 mm 单件（同机 Python 五步）102 s → **3.6 s**；峰值内存 11.5 GB → **0.83 GB**
- ⚠ **发布只需要 `bin/` 里这一个动态库**（`fry-rip-cli/` 子进程目录没有了）；`FryHalftone_SetWorkerPathUtf8` / `GetWorkerPathUtf8` 保留符号但不再起作用
- 新增 `FryHalftone_GetEngine()`（固定 `"cpp"`）与 `FryHalftone_GetParamSchemaJson()`（参数白名单 JSON）
- 新增返回码 `FRY_HALFTONE_UNSUPPORTED (7)`：模型用到 C++ 版暂不支持的特性（渐进式 / 12 位 / CMYK JPEG、外链贴图、稀疏 accessor、
  非三角形图元、实例化节点、近刚体矩阵、细分规模超阈值需 ray 路线）—— 绝不静默出一个与 Python 不一致的结果
- ★ **恢复两个 383 删掉的参数并真正实现**：
  - `use_black_k` + `ink_k_um` + `black_k_amount`：CMYK 四通道，灰分量 min(C,M,Y)×强度 挪给 K 墨（本机暂无 K 墨，为将来保留）
  - `color_thickness_mm`：色带厚度；`null` = 各墨满喷厚度之和（C+M+Y[+K]）；改变墨往多深摊 —— 低于本件墨量需求会截断（变浅、偏色），
    调大在 2 倍默认以内正对视角几乎不变（算法仓 384_13 实测：它不是明度旋钮）
- 新增 `threads` 参数；结果 JSON 新增 `engine` / `inkedVoxels` / `bmpMd5` / `bandThicknessMm` / `colorChannels` / `elapsedSeconds`
- 支持基线 JPEG 贴图（逐位复刻 Pillow + libjpeg-turbo）与任意层级节点变换（matrix / TRS，复刻 trimesh + OpenBLAS 的乘法顺序）
- 坏文件（截断 / 越界 / NaN / 坐标溢出）返回 1 不崩不卡：fuzz 17.4 万次（第一轮撞出一个死循环 —— 节点平移 −1e308 ⇒ 坐标溢出 ⇒ 最近面搜索半径无限倍增 —— 已修）；
  工作线程里的异常带回调用线程再翻成返回码（异常逃出线程 = 宿主进程被 terminate）
- Windows「混合 CRT」：vcruntime / msvcp 静态链入（对接方不必装 VC++ 运行库），UCRT 动态链系统 `ucrtbase.dll`（与 numpy 同一份 `pow` / `atan2`）
- 编译纪律：GCC / Clang `-ffp-contract=off -fno-fast-math`（⚠ Apple arm64 默认收缩成 FMA，不关必然与 Python 不同）；MSVC `/fp:precise`
- ★ **CI #2854243250 修复**：载荷闸报「被改 427」实为换行符 —— 本机 `core.autocrlf=true`（CRLF）算的锁，Linux runner 检出 LF。
  指纹改为文本 CRLF→LF 后再哈希 + 仓库 `.gitattributes`（`tests/test_payload_gate.py` 钉住 LF / CRLF 两种检出）
- 删除：`python/`、`payload/`、`payload.lock.json`、`requirements-sdk.txt`、`tests/compare_frozen_vs_source.py`、`tests/test_cli_contract.py`
- 新增：`tools/fry_deliver.py`（dll 交付进算法仓界面组件 HS200）、`tests/test_golden.py` + `tests/data/`

### 383 参数契约收紧（接口形状不变；算法载荷不变，指纹仍是 `a58817e8…`）
- ⚠⚠ **尺寸口径改为与 lmst 一致**：`targetHeightMm` / `target_height_mm` = 成品 **z 高度**（包围盒摆正后最短边朝 z，再等比缩放）；
  新增 `target_longest_edge_mm` = 最长边（= 算法仓流程表 `TARGET_MM`）。**二选一、必须给**（尺寸不再有默认值 16.851）。
  —— 382 版按最长边缩放，与 lmst（`CpuSlicePipeline.cpp::normalizeMeshToTargetHeight`：`scale = targetHeightMm / z 高度`）同名不同义：
  立方体上一样，实测 2×1×0.5 mm 长方体上差 4 倍（z 1 mm = 109 张 vs z 0.25 mm = 33 张），全程不报错
- 仍与 lmst 不同的一处：lmst 用 PCA 摆正，本 SDK 按包围盒轴摆正（斜放的模型两边 z 高度会不同）
- **参数严格按类型校验**：类型不对返回 2 并说明 —— 修之前 `"support_enable": "false"`（字符串）被当成「开」
- 移除两个参数（★384 已按需求恢复，见上）：`color_thickness_mm`、`use_black_k`
- `slice_metadata.json` 新增 `params_dict`（本次生效的参数）与 `size_dict`（尺寸按哪种口径换算、成品最长边与 z 高度）
- C 接口 `targetHeightMm` 为 NaN / 无穷大时直接返回 2「必须是有限的数字」

### 382 首版（Python 子进程版，★384 已退役）
- 水转印加网（`water_transfer_depth`）SDK 首版：C ABI（`FryHalftone_*`），形状与 lmst 的 `LmstYzHalftone_*` 一致，另有 JSON 参数接口
- 算法进程 `fry-rip-cli`（PyInstaller 冻结）；七步出包流水线；载荷副本 + 逐文件 sha256 锁文件 + 载荷闸；两份 CI
- 半球 16.851 mm：打包后的 dll 与算法仓逐字节相同 852 / 852（填透明、填白）
