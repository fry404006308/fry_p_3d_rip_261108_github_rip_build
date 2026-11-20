/*
 * fry_halftone.h —— 水转印加网 SDK 的 C 接口（跨仓契约）
 * FryHalftone SDK — water-transfer 3D halftone, C ABI.
 *
 * ---------------------------------------------------------------------------
 * 这个库是做什么的 / What it does
 *   输入：一个彩色 GLB 模型                 in : one textured/coloured .glb model
 *   输出：一叠 4-bit 调色板 BMP 切片         out: a stack of 4-bit palette BMP slices
 *         （000000.bmp 起，一张 = 一个打印层）    (000000.bmp …, one file per print layer)
 *         外加 slice_metadata.json + _orient.json   plus slice_metadata.json and _orient.json
 *   算法：水转印 + 深度抖动（water_transfer_depth）—— 本 SDK **只提供这一种**。
 *
 * ---------------------------------------------------------------------------
 * 怎么用（最短路径）/ Minimal usage
 *   1. LoadLibrary("fry_halftone.dll") / dlopen("libfry_halftone.so")
 *   2. FryHalftone_GetVersion()                     看看拿到的是哪一版
 *   3. FryHalftone_SliceModel(model, outDir, 0, cb, user, err, 1024)
 *   4. 返回 0 = 成功；非 0 读 errorBuffer
 *
 * ---------------------------------------------------------------------------
 * ⚠ 必须知道的事 / Things you must know
 *   ① ★384 起算法是 **C++、在调用方进程内**跑（382/383 是 Python 子进程）。发布只需要这一个动态库。
 *      结果与算法仓 Python 生产线逐字节一致（半球、灰阶球、K 墨、色带厚度、JPEG + 节点变换等用例，见 tests/test_golden.py；
 *      算法仓 384 另有 21 个真实 / 合成模型的 23 项中间结果全量对账）。
 *      The algorithm now runs in-process (C++); only this one shared library is needed.
 *   ② 内存：半球件（17 mm）峰值约 0.83 GB（384 实测，同机 Python 11.5 GB）；随体素数线性增长（体素数 ≈ 三边 mm ÷ (0.137, 0.035, 0.01) 相乘）。
 *   ③ 同一个库实例**同一时刻只跑一个作业**；第二个并发调用返回 FRY_HALFTONE_BUSY。
 *   ④ 进度回调在**调用 SliceModel 的那个线程**上同步触发；回调里别做耗时操作。
 *      要中止就从**另一个线程**调 FryHalftone_Cancel()，作业在下一个检查点（各步之间、写 BMP 每 32 层）停下。
 *   ⑤ 支持：内嵌 PNG、基线 JPEG（1/3 分量）、任意层级节点变换（matrix / TRS，含镜像）、贴图 / 顶点色 / 材质色混合。
 *      ★391 颜色按 glTF 规范：baseColorFactor × 贴图 × COLOR_0（缺哪项取 1；材质没写 factor = 白），
 *      TEXCOORD_n + KHR_texture_transform、wrapS/wrapT（重复 / 夹边 / 镜像）、magFilter NEAREST、高光光泽度工作流；
 *      顶点色 float / u8 / u16；三角条带、三角扇、稀疏 accessor、量化坐标与 uv（KHR_mesh_quantization）、data URI / 同目录外链贴图。
 *      暂不支持的模型返回 FRY_HALFTONE_UNSUPPORTED(7)，errorBuffer 写清是哪一种 —— 绝不静默出一个与 Python 不一致的结果：
 *      渐进式 / 12 位 / CMYK JPEG、WebP / KTX2 等没有 PNG/JPEG 回退的贴图、Draco / meshopt 压缩、GPU 实例化、
 *      声明必需而本库不认识的扩展、GLB 里带 uri 的 buffer、.gltf 文本文件、一个节点多父、近刚体矩阵（trimesh 会 SVD 修正）。
 *      ★390 起体素化走 exact 精确路线（两边同式、逐位一致）⇒ **不再因为模型大而返回 7**；大件只受内存限制（见 ②）。
 *      坏文件（截断 / 越界 / NaN 坐标）返回 FRY_HALFTONE_FAILED(1)，不会让宿主进程崩溃。
 * ---------------------------------------------------------------------------
 */
#ifndef FRY_HALFTONE_H
#define FRY_HALFTONE_H

#include <wchar.h>

/* ⚠ 这份头文件**既要能被 C++ 包含，也要能被纯 C 包含**（消费方可能是 C、C#、Delphi 的 P/Invoke
 * 声明生成器，甚至只是 grep 这个文件）。`extern "C"` 在 C 编译器里是语法错误，所以必须分开写。*/
#ifdef __cplusplus
#  define FRY_EXTERN_C extern "C"
#else
#  define FRY_EXTERN_C
#endif

#ifdef _WIN32
#  ifdef FRY_HALFTONE_BUILDING
#    define FRY_API FRY_EXTERN_C __declspec(dllexport)
#  else
#    define FRY_API FRY_EXTERN_C __declspec(dllimport)
#  endif
#  define FRY_CALL __stdcall
#elif defined(__GNUC__) || defined(__clang__)
#  define FRY_API FRY_EXTERN_C __attribute__((visibility("default")))
#  define FRY_CALL
#else
#  define FRY_API FRY_EXTERN_C
#  define FRY_CALL
#endif

/* ── 返回码 / Return codes ───────────────────────────────────────────────── */
#define FRY_HALFTONE_OK             0   /* 成功                                        success            */
#define FRY_HALFTONE_FAILED         1   /* 算法跑了但失败了（读 errorBuffer）          algorithm failed   */
#define FRY_HALFTONE_BAD_ARGUMENT   2   /* 参数不合法，一步都没跑                      bad argument       */
#define FRY_HALFTONE_NO_WORKER      3   /* （382/383 子进程时代；384 起不会再返回）    legacy             */
#define FRY_HALFTONE_PROTOCOL       4   /* （382/383 子进程时代；384 起不会再返回）    legacy             */
#define FRY_HALFTONE_CANCELLED      5   /* 被 FryHalftone_Cancel 中止                  cancelled          */
#define FRY_HALFTONE_BUSY           6   /* 已经有一个作业在跑                          another job running*/
#define FRY_HALFTONE_UNSUPPORTED    7   /* 模型用到了 C++ 版暂不支持的特性（见 ⑤）     unsupported input  */

/* ── 进度回调 / Progress callbacks ──────────────────────────────────────────
 * percent 单调不减，0~100；stage 是一句人读的中文（UTF-16 / UTF-8 两版）。
 * 回调可以传 NULL 表示不要进度。                                              */
typedef void(FRY_CALL* FryProgressCallback)(int percent, const wchar_t* stage, void* userData);
typedef void(FRY_CALL* FryUtf8ProgressCallback)(int percent, const char* stage, void* userData);

/* ── 自报家门 / Identity ─────────────────────────────────────────────────────
 * 返回的都是库内静态 UTF-8 字符串，**不要 free**。
 * ★411 交付版返回 "X.Y.Z"（0.1.0 起，每次交付递增；CHANGELOG 按版本号分节）；
 * 本地调试构建（没指定版本号）返回占位值 "0.0.0-dev"（与 lmst 同口径），这种构建不许交付。 */
FRY_API const char* FRY_CALL FryHalftone_GetVersion(void);

/* 算法载荷指纹（64 位十六进制 sha256，= 打包仓 cpp.lock.json 的 tree_sha256）：这个库里编的到底是哪一版算法源码。 */
FRY_API const char* FRY_CALL FryHalftone_GetAlgorithmFingerprint(void);

/* 本 SDK 提供的加网算法名，固定 "water_transfer_depth"。                      */
FRY_API const char* FRY_CALL FryHalftone_GetAlgorithmName(void);

/* ★384 算法引擎："cpp"（382/383 的 Python 子进程版没有这个函数）。             */
FRY_API const char* FRY_CALL FryHalftone_GetEngine(void);

/* ★384 参数白名单（JSON）—— 桌面端可以据此生成设置界面。
 * ★387 升到 schema = fry-halftone-params/3：每个参数除 name / kind / description 外，还有
 *   default（省略该键时库真正用的值）、nullable，以及按类型给出的 min / min_exclusive / max / integer / choices / length；
 *   顶层另有 one_of_required（尺寸二选一）与 requires（use_black_k=true 时必须给 ink_k_um）。
 *   出包时原样导出为 SDK 包里的 params_schema.json（人读版 SDK_参数说明.md 由它生成），冒烟时真调库核对过默认值与收拒范围。 */
FRY_API const char* FRY_CALL FryHalftone_GetParamSchemaJson(void);

/* ── 主接口（与 lmst 的 LmstYzHalftone_* 形状一致，便于桌面端照搬）──────────
 * inputModelPath       输入 .glb
 * outputSliceDirectory 输出目录（不存在会建；BMP 直接落在这一层）
 * targetHeightMm       成品 **z 高度**（毫米）：模型先按包围盒摆正（最短边朝 z），再等比缩放到 z 高度 = 这个数；
 *                      **0 = 保持模型自带的毫米尺寸**。与 lmst 的 targetHeightMm 同义（lmst 用 PCA 摆正，这里按包围盒轴）。
 *                      其余参数用 SDK 默认值（C 400 / M 500 / Y 300 µm、无 K 墨、色带厚度 = 满喷之和、填透明、打支撑）。
 * errorBuffer          失败时写入原因（UTF-16 / UTF-8），可传 NULL
 * errorBufferChars     errorBuffer 能放多少个 wchar_t（Utf8 版是多少字节）
 * 返回 0 成功，非 0 见上面的返回码。                                          */
FRY_API int FRY_CALL FryHalftone_SliceModel(const wchar_t* inputModelPath,
                                            const wchar_t* outputSliceDirectory,
                                            double targetHeightMm,
                                            FryProgressCallback progressCallback,
                                            void* userData,
                                            wchar_t* errorBuffer,
                                            int errorBufferChars);

FRY_API int FRY_CALL FryHalftone_SliceModelUtf8(const char* inputModelPath,
                                                const char* outputSliceDirectory,
                                                double targetHeightMm,
                                                FryUtf8ProgressCallback progressCallback,
                                                void* userData,
                                                char* errorBuffer,
                                                int errorBufferBytes);

/* ── 可扩展接口：参数走 JSON，**以后加参数不用再改 ABI** ─────────────────────
 * requestJson（UTF-8）：
 *   {
 *     "model":  "D:/x/a.glb",            必填
 *     "out":    "D:/x/slices",           必填
 *     "params": {                        ★必填（至少要有尺寸）；键必须在白名单里、类型必须对（写错直接返回 2，不静默忽略）
 *        "target_height_mm":  30.0,      ┐ 尺寸二选一、必须给一个：z 高度 mm（0 = 保持原尺寸）
 *        "target_longest_edge_mm": null, ┘                        ｜ 最长边 mm（= 算法仓 TARGET_MM）
 *        "ink_c_um": 400,                数字 ≥0：C 满喷厚度 µm
 *        "ink_m_um": 500,                数字 ≥0：M 满喷厚度 µm
 *        "ink_y_um": 300,                数字 ≥0：Y 满喷厚度 µm
 *        "ink_k_um": null,               ★384 数字 ≥0 或 null：K 满喷厚度 µm（use_black_k=true 时必须给）
 *        "use_black_k": false,           ★384 布尔：true ⇒ CMYK 四通道，灰分量 min(C,M,Y)×black_k_amount 挪给 K 墨
 *        "black_k_amount": 1.0,          ★384 数字 [0,1]：灰分量替换强度
 *        "color_thickness_mm": null,     ★384 数字 >0 或 null：色带厚度 mm；null = 各墨满喷厚度之和（C+M+Y[+K]）
 *                                          调小 ⇒ 墨往表层集中（低于本件墨量需求时被截断：变浅、偏色）；
 *                                          调大 ⇒ 墨往深处摊（正对视角颜色变化很小，过厚会损失清晰度）。调明度请调墨量，不要调它（384 实测）
 *        "band_uninked_fill": "transparent",  只认 "transparent" | "white"
 *        "support_enable": true,         必须是 JSON 布尔 true/false（字符串 "false" 会被拒绝）
 *        "output_width_px": 128,         正整数或 null（null = 按模型实际宽度）
 *        "voxel_mm": [0.137, 0.035, 0.01],    三个正数：墨滴尺寸 mm (x, y, z)，z 就是层高
 *        "threads": null,                非负整数或 null（null/0 = 全部硬件线程）
 *        "orient_mode": "auto_extent",   ★392 加网方向：
 *                                          "auto_extent"（默认）= 按边长自动摆正（最长→x、次长→y、最短→z）——
 *                                            它只定「哪条边配哪根轴」，不定正负；实测等价于 axis_map 的 up="+short", front="+mid"；
 *                                          "original"（0.4.0 起）= 保留节点变换后的 GLB 场景方向；yaw=0，尺寸仍按请求缩放。
 *                                          "axis_map" = 用下面两根**有符号轴**唯一定死朝向（6 × 4 = 24 种，det=+1 不镜像）
 *        "orient_up": "+short",          模型的哪条有符号轴朝上 ⇒ voxel +z（逐层方向；BMP 张数 = 这条边 / 层厚）
 *        "orient_front": "+mid",         模型的哪条有符号轴朝正面 ⇒ voxel +y（运行方向）；voxel +x（喷孔方向）= front × up 自动定
 *                                          两个轴名都只认：+x -x +y -y +z -z（模型自身轴）／ +long -long +mid -mid +short -short（包围盒边长）
 *        "orient_yaw_deg": 0.0,          axis_map 时再绕 voxel z 转的角度；90° 的整数倍是精确值，
 *                                          其它角度模型斜放（包围盒变大、张数与宽度都会变）
 *        ── ★449 水转印统一引擎（HE500 / HE510 / HE520 一份代码）。不给 wt_engine = 原 HE500，结果与 0.1.x 逐字节相同 ──
 *        "wt_engine": "legacy",          只认 "legacy"（默认，原 HE500）| "unified"（统一代码）；下面 8 个键**只在 unified 时许出现**
 *        "wt_preset": "he500",           unified 时：he500 | he510（全外移）| he520（光学外移）| custom（四个开关自己拼）
 *        "wt_outward_columns": "normal", he510 / he520 时：normal（法线柱，默认）| edt（老 EDT 柱，复现 428~435 用）
 *        "wt_outward_strength": 0.5,     he520 / custom 时：外移强度 [0,1]
 *        "wt_outward_attenuation_mm_inv": 4.0,  he520 / custom 时：衰减代理 λ（1/mm），(0, 1000]
 *        "wtu_columns": "normal",        custom 时：edt | normal
 *        "wtu_ray_origin": "voxel",      custom 时：voxel（表面体素中心）| surface_point（曲面最近点）
 *        "wtu_move": "optical",          custom 时：none | compact | optical
 *        "wtu_dither": "blue_noise"      custom 时：blue_noise | none
 *                                          给了却用不上的键（如 he510 + wt_outward_strength）返回 2，不静默忽略；
 *                                          规则机读版 = params_schema.json 的 only_when；legacy 作业的回显里不出现这 9 个键（echo_omit_when）
 *     }
 *   }
 * resultJson（UTF-8）：{"success":true,"outputDirectory":…,"sliceCount":…,"metadataPath":…,"algorithm":…,"engine":"cpp",
 *                      "sdkVersion":…,"algorithmFingerprint":…,"inkedVoxels":…,"bmpMd5":…,"bandThicknessMm":…,
 *                      "colorChannels":…,"params":{…},"size":{…},"elapsedSeconds":…}
 * ⚠ resultJsonBytes 不够时返回 FRY_HALFTONE_BAD_ARGUMENT 并把**需要的字节数**写进 errorBuffer。建议给 8192 字节。 */
FRY_API int FRY_CALL FryHalftone_SliceModelJsonUtf8(const char* requestJson,
                                                    char* resultJson,
                                                    int resultJsonBytes,
                                                    FryUtf8ProgressCallback progressCallback,
                                                    void* userData,
                                                    char* errorBuffer,
                                                    int errorBufferBytes);

/* ── 中止 / Cancel ──────────────────────────────────────────────────────────
 * 置中止标志；正在跑的那次调用在下一个检查点返回 FRY_HALFTONE_CANCELLED（已写出的 BMP 不删）。
 * 没有作业在跑时返回 FRY_HALFTONE_OK（幂等）。可以从任意线程调用。            */
FRY_API int FRY_CALL FryHalftone_Cancel(void);

/* ── 382/383 子进程时代的两个函数：★384 起没有子进程，保留符号只为 ABI 兼容 ──
 * SetWorkerPathUtf8 什么都不做、返回 OK；GetWorkerPathUtf8 返回空串 ""。        */
FRY_API int FRY_CALL FryHalftone_SetWorkerPathUtf8(const char* workerPath);
FRY_API const char* FRY_CALL FryHalftone_GetWorkerPathUtf8(void);

#endif /* FRY_HALFTONE_H */
