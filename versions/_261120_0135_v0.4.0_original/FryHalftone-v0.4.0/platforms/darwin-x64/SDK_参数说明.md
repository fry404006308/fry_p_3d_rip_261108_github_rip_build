# 水转印加网 SDK · 参数说明 / Parameter reference

> ⚠ **本文件由出包脚本自动生成，不要手改。** 来源：这个动态库自己导出的参数说明 `FryHalftone_GetParamSchemaJson()`（= 同目录 `params_schema.json`）。
> 出包冒烟已真调动态库核对过：**省略一个参数 = 用这里写的默认值**（结果逐字节相同）；**取值范围的收 / 拒与这里一致**。
> Generated at packaging time from the library's own schema; defaults and ranges were verified against the library.

| 项 / item | 值 / value |
| --- | --- |
| SDK 版本 / version | `0.4.0` |
| 平台 / platform | `darwin-x64` |
| 说明版本 / schema | `fry-halftone-params/3` |
| 参数个数 / parameters | 27 |
| 描述的动态库 sha256 / library sha256 | `f827aca236257801703f73809d50ec17aabd09b37f72aeb55856f3bf9e0ddf0d` |

## 一、参数表 / Parameters

| # | 参数 / name | 类型 / type | 默认值（省略时真正用的值）/ default | 取值范围 / range | 说明 / description |
| ---: | --- | --- | --- | --- | --- |
| 1 | `target_height_mm` | 数字 | `null` | ≥ 0.0 或 `null` | 成品 z 高度 mm（摆正后最短轴朝 z）；0 = 保持模型自带尺寸；与 target_longest_edge_mm 二选一 |
| 2 | `target_longest_edge_mm` | 数字 | `null` | > 0.0 或 `null` | 成品最长边 mm（= 算法仓 TARGET_MM）；与 target_height_mm 二选一 |
| 3 | `ink_c_um` | 数字 | `400.0` | ≥ 0.0 | C 满喷厚度 µm（默认 400） |
| 4 | `ink_m_um` | 数字 | `500.0` | ≥ 0.0 | M 满喷厚度 µm（默认 500） |
| 5 | `ink_y_um` | 数字 | `300.0` | ≥ 0.0 | Y 满喷厚度 µm（默认 300） |
| 6 | `ink_k_um` | 数字 | `null` | ≥ 0.0 或 `null` | K 满喷厚度 µm；use_black_k=true 时必须给 |
| 7 | `use_black_k` | 布尔 | `false` | `true` / `false` | true ⇒ CMYK 四通道（灰分量挪给 K 墨） |
| 8 | `black_k_amount` | 数字 | `1.0` | ≥ 0.0，≤ 1.0 | 灰分量替换强度 [0,1]（默认 1） |
| 9 | `color_thickness_mm` | 数字 | `null` | > 0.0 或 `null` | 色带厚度 mm；null = 各墨满喷厚度之和 |
| 10 | `band_uninked_fill` | 字符串（枚举） | `"transparent"` | `transparent` / `white` | 色带里不落彩墨的格子填什么：transparent / white |
| 11 | `support_enable` | 布尔 | `true` | `true` / `false` | 是否打支撑 |
| 12 | `output_width_px` | 整数 | `null` | 整数 ≥ 1 或 `null` | 输出 BMP 宽度（喷头数）；null = 按模型实际宽度 |
| 13 | `voxel_mm` | 数组（3 个数） | `[0.137, 0.035, 0.01]` | 3 个数，每个 > 0.0 | 墨滴尺寸 mm [x, y, z]；z = 层高 |
| 14 | `orient_mode` | 字符串（枚举） | `"auto_extent"` | `auto_extent` / `axis_map` / `original` | ★392 加网方向：auto_extent（按边长自动摆正，392 前的老行为）/ axis_map（指定朝上 + 正面）/ original（保持 GLB 场景原始 XYZ） |
| 15 | `orient_up` | 字符串（枚举） | `"+short"` | `+x` / `-x` / `+y` / `-y` / `+z` / `-z` / `+long` / `-long` / `+mid` / `-mid` / `+short` / `-short` | axis_map 时：模型的哪条有符号轴朝上（= voxel +z，逐层方向；BMP 张数 = 这条边 / 层厚） |
| 16 | `orient_front` | 字符串（枚举） | `"+mid"` | `+x` / `-x` / `+y` / `-y` / `+z` / `-z` / `+long` / `-long` / `+mid` / `-mid` / `+short` / `-short` | axis_map 时：模型的哪条有符号轴朝正面（= voxel +y，运行方向）；喷孔方向 x = front × up 自动定 |
| 17 | `orient_yaw_deg` | 数字 | `0.0` | 任意 | axis_map 时：定好朝上/正面后再绕 voxel z 转多少度（90 的整数倍用精确值） |
| 18 | `threads` | 整数 | `null` | 整数 ≥ 0 或 `null` | 线程数；null / 0 = 全部硬件线程 |
| 19 | `wt_engine` | 字符串（枚举） | `"legacy"` | `legacy` / `unified` | ★448 水转印引擎：legacy = 原 HE500 C++（默认，一字未改）/ unified = 统一代码（HE500/HE510/HE520） |
| 20 | `wt_preset` | 字符串（枚举） | `"he500"` | `he500` / `he510` / `he520` / `custom` | unified 时：he500 / he510（全外移）/ he520（光学外移，默认强度 0.5）/ custom（四个开关自己拼） |
| 21 | `wt_outward_columns` | 字符串（枚举） | `"normal"` | `normal` / `edt` | unified + he510/he520：柱口径 normal（437 法线柱，默认）/ edt（428~435 老 EDT 柱） |
| 22 | `wt_outward_strength` | 数字 | `0.5` | ≥ 0.0，≤ 1.0 | unified + he520/custom：外移强度 a ∈ [0,1]（0 = 不挪 = HE500，1 = 全挤到柱头；默认 0.5） |
| 23 | `wt_outward_attenuation_mm_inv` | 数字 | `4.0` | > 0.0，≤ 1000.0 | unified + he520/custom：衰减代理 λ（1/mm），(0, 1000]，默认 4 |
| 24 | `wtu_columns` | 字符串（枚举） | `"normal"` | `edt` / `normal` | unified + custom：柱 edt（EDT 最近表面）/ normal（沿法线射线，默认） |
| 25 | `wtu_ray_origin` | 字符串（枚举） | `"voxel"` | `voxel` / `surface_point` | unified + custom + normal 柱：射线起点 voxel（表面体素中心，默认）/ surface_point（曲面最近点） |
| 26 | `wtu_move` | 字符串（枚举） | `"optical"` | `none` / `compact` / `optical` | unified + custom：外移 none（不挪）/ compact（全挤到柱头）/ optical（指数目标深度，默认） |
| 27 | `wtu_dither` | 字符串（枚举） | `"blue_noise"` | `blue_noise` / `none` | unified + custom + optical：取整抖动 blue_noise（默认）/ none |

## 二、跨参数规则 / Cross-parameter rules

- **必须且只能给一个**（one of, required）：`target_height_mm` / `target_longest_edge_mm`
- `use_black_k` = `true` 时**必须同时给** `ink_k_um`
- `wt_preset`、`wt_outward_columns`、`wt_outward_strength`、`wt_outward_attenuation_mm_inv`、`wtu_columns`、`wtu_ray_origin`、`wtu_move`、`wtu_dither` **只在** `wt_engine` ∈ `unified` 时许出现（否则返回 2）
- `wtu_columns`、`wtu_ray_origin`、`wtu_move`、`wtu_dither` **只在** `wt_preset` ∈ `custom` 时许出现（否则返回 2）
- `wt_outward_columns` **只在** `wt_preset` ∈ `he510` / `he520` 时许出现（否则返回 2）
- `wt_outward_strength`、`wt_outward_attenuation_mm_inv` **只在** `wt_preset` ∈ `he520` / `custom` 时许出现（否则返回 2）
- `wt_engine` ∈ `legacy` 时，结果里的参数回显**不出现** `wt_engine`、`wt_preset`、`wt_outward_columns`、`wt_outward_strength`、`wt_outward_attenuation_mm_inv`、`wtu_columns`、`wtu_ray_origin`、`wtu_move`、`wtu_dither`（与旧版回显逐字节相同）

## 三、怎么用 / How to use

- 参数走 `FryHalftone_SliceModelJsonUtf8` 的 `params` 对象；**键不在上表里、类型或范围不对 ⇒ 返回 2（BAD_ARGUMENT），不会静默忽略**。
- 只想用默认值：`params` 里只放尺寸即可，例如 `{"target_height_mm": 30.0}`。
- 做设置界面：读 `params_schema.json`（字段 `name / kind / description / default / nullable / min / min_exclusive / max / integer / choices / length`），不要照抄本表 —— 换 dll 后以新 dll 自报为准。
