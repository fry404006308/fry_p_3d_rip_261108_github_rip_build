# v0.2.0 的全部 27 个参数

一句话：从该批动态库调用 FryHalftone_GetParamSchemaJson 得到下表，已经逐平台与原包参数清单核对。用于构建设置界面或请求编码器时，直接读取 [机器可读原表](../platforms/win32-x64/params_schema.json)。

|键|类型|省略时默认|范围或枚举|实际作用|
|---|---|---|---|---|
|`target_height_mm`|number_or_null|`null`|≥0.0，可为 null|摆正后沿打印机 Z 缩放；0 保持模型毫米尺寸。与最长边只选一个有效值。|
|`target_longest_edge_mm`|number_or_null|`null`|>0.0，可为 null|按最长边等比缩放，例如 16.0 mm。不能传 0。|
|`ink_c_um`|number|`400.0`|≥0.0|C 满喷厚度 µm（默认 400）|
|`ink_m_um`|number|`500.0`|≥0.0|M 满喷厚度 µm（默认 500）|
|`ink_y_um`|number|`300.0`|≥0.0|Y 满喷厚度 µm（默认 300）|
|`ink_k_um`|number_or_null|`null`|≥0.0，可为 null|K 满喷厚度 µm；use_black_k=true 时必须给|
|`use_black_k`|bool|`false`|true / false|true ⇒ CMYK 四通道（灰分量挪给 K 墨）|
|`black_k_amount`|unit_interval|`1.0`|≥0.0，≤1.0|灰分量替换强度 [0,1]（默认 1）|
|`color_thickness_mm`|number_or_null|`null`|>0.0，可为 null|自动时用启用通道的满喷厚度之和；缩小窗口可能截断需求，改变深浅或色相。|
|`band_uninked_fill`|choice_fill|`"transparent"`|transparent / white|色带未落彩墨处填白或透明。白、透明都属于喷印材料。|
|`support_enable`|bool|`true`|true / false|生成向下支撑。v0.2.0 参数表没有 HR100 的全包裹开关。|
|`output_width_px`|int_or_null|`null`|≥1，可为 null|固定 X 宽，只向右补空。设备为 128 喷孔时传 128，一旦超宽会报错。|
|`voxel_mm`|vec3_positive|`[0.137,0.035,0.01]`|长度 3，各分量 > 0|三个轴依次为喷孔、运行、逐层，每项 > 0。层数折算用第三项。|
|`orient_mode`|choice_orient_mode|`"auto_extent"`|auto_extent / axis_map|默认按包围盒排序；axis_map 使用给定朝上、正面及 yaw。|
|`orient_up`|choice_axis|`"+short"`|+x / -x / +y / -y / +z / -z / +long / -long / +mid / -mid / +short / -short|axis_map 的模型有符号轴映射到打印机 +Z。short/mid/long 取模型包围盒边长排名。|
|`orient_front`|choice_axis|`"+mid"`|+x / -x / +y / -y / +z / -z / +long / -long / +mid / -mid / +short / -short|axis_map 的模型有符号轴映射到 +Y；与朝上轴需实际垂直。余下的 +X 由 front × up 决定。|
|`orient_yaw_deg`|number_any|`0.0`|数值|绕打印机 Z 轴旋转。auto_extent 只能为 0；45° HE510 跨平台特例见测试报告。|
|`threads`|int_or_null|`null`|≥0，可为 null|每个作业内的 CPU 线程；服务端宜显式分配。实现将显式正整数上限钳到 1024。|
|`wt_engine`|choice|`"legacy"`|legacy / unified|默认 legacy。需要 HE510/HE520/custom 或统一 HE500 时发送 unified。|
|`wt_preset`|choice|`"he500"`|he500 / he510 / he520 / custom|选 unified 后出现；he500、he510、he520、custom 四选一。|
|`wt_outward_columns`|choice|`"normal"`|normal / edt|仅 he510 / he520 可传；normal 沿曲面法线，edt 用最近表面格划柱。|
|`wt_outward_strength`|unit_interval|`0.5`|≥0.0，≤1.0|仅 he520 / custom 可传。0 到 1 调整同一柱内的移位，不同预设的柱和起点也会改变结果，端点不能单独保证预设等价。|
|`wt_outward_attenuation_mm_inv`|attenuation|`4.0`|>0.0，≤1000.0|仅 he520 / custom 可传。单位 mm⁻¹，控制深度权重。它是外移算法参数。|
|`wtu_columns`|choice|`"normal"`|edt / normal|仅 custom 可传，决定色带内哪些格组成同一柱。|
|`wtu_ray_origin`|choice|`"voxel"`|voxel / surface_point|仅 custom 可传，normal 柱才用。voxel 从表面体素中心，surface_point 从几何最近点起。|
|`wtu_move`|choice|`"optical"`|none / compact / optical|仅 custom 可传。none 保持深度分布，compact 保序移到柱头，optical 按目标深度柔移。|
|`wtu_dither`|choice|`"blue_noise"`|blue_noise / none|仅 custom 可传，optical 移动时控制连续深度到整数格的取整方式。|

## 写请求时的顺序

1. 顶层只传 model、out、params。model 和 out 是非空字符串。两个尺寸键选择其中一个，另一个省略。JSON 布尔值使用 true/false。遇到未知键、类型不符、非法组合，将原始错误返回给调用方。
2. 根据这批墨给各通道满喷合同。0.010 mm 层高下，400/500/300 µm 分别对应 40/50/30 层。改用 0.020 mm 时变为 20/25/15 层。K 默认关闭；打开须显式传 ink_k_um。启用的彩墨满喷厚度合计须大于零。
3. 用户确定摆放方向后，再计算尺寸、宽度和层数。auto_extent 搭配 yaw=0。axis_map 先选互相垂直的 up/front，再决定 yaw。混用边长名与模型轴名时应检查解析后是否共线。
4. 先选 wt_engine；legacy 时省略其余八个引擎字段。unified 时再按预设选下表字段。显示在响应中的默认字段可能只是回显，对应输入仍受 only_when 限制；不要将整段回显原样当下一次请求。

|预设|发送示例（在 params 已有尺寸字段的基础上增加）|识别输出|
|---|---|---|
|传统 HE500|省略 wt_engine 与后续八项|water_transfer_depth|
|统一 HE500|wt_engine=unified，wt_preset=he500|water_transfer_unified_he500|
|HE510 保序紧压|wt_engine=unified，wt_preset=he510，可选 wt_outward_columns|water_transfer_unified_he510|
|HE520 柔移|wt_engine=unified，wt_preset=he520，可选柱、强度、衰减|water_transfer_unified_he520|
|自定义 HE530|wt_engine=unified，wt_preset=custom，可选 wtu_* 及外移强度、衰减|water_transfer_unified|

custom 下，wtu_ray_origin 在法线柱中有物理意义，wtu_dither 在 optical 移动中有意义。表单应只提交当前路径使用的键。SDK 对跨预设传错键会拒绝，例如 he510 + wt_outward_strength。当前样例在 Windows/WSL 均实测该组合 CLI 返回 2。

## 怎样选首个参数

先用传统或统一 HE500 建参考图。需要把一根柱里的彩墨连续排向表面，选 HE510；需要保留部分深度变化，选 HE520，先用 0.5 和 4.0 mm⁻¹。对比同一模型时锁定填充、满喷厚度、间距和姿态。墨总数相近仍可能分布不同，以逐层结果核对。
