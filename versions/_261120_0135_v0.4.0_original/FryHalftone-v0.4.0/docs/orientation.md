# 保持 GLB 原始朝向
HR100 与 HS200 均在「模型方向 → 加网方向」中选择「original：保持 GLB 原始朝向」。输入仍是包含 GLB 的文件夹。默认 auto_extent 沿用现有作业设置。切换前如设置过非零平面旋转，请将其设为 0。

|模式|打印 XYZ 的含义|适用作业|
|---|---|---|
|original|逐轴等于 GLB 展开节点变换后的场景 XYZ|客户已经摆好的模型，或需要与设计坐标对齐|
|auto_extent|按包围盒边长自动摆放|优先减少层数等传统作业|
|axis_map|由有符号的朝上轴、正面轴、平面旋转指定|需要自定义摆放的模型|

原始局部网格仍先应用各自节点的变换；尺寸口径、居中和栅格留白仍由既有流程处理。GLB 场景 +Z 直接对应机器逐层方向，即使查看器采用 +Y 朝上。若需要把查看器的 +Y 转成打印机 +Z，请用 axis_map。original 下元数据矩阵为三阶单位阵。非零 yaw 请求将被拒绝。

新版库正式接受 `orient_mode="original"`。旧 v0.2.0 库同样可以得到单位方向，可发送 `axis_map`、`orient_up="+z"`、`orient_front="+y"`、`orient_yaw_deg=0`，其 CLI 的 --no-auto-orient 已经采用此组合。工作台在手动指定该旧库时能进行相同转换。

## 亲手复现

在产品根目录执行 `platforms/win32-x64/bin/fry-halftone-cli.exe run examples/original.json --json`。Linux/macOS 改用对应架构的 bin/fry-halftone-cli。输出目录 first-original 须为空或不存在。样例的局部盒边长为 3、1、2 mm，节点旋转加平移后场景尺寸为 2、1、3 mm。最长边设 3 mm：原始模式产 309 层，自动模式产 109 层。检查 _orient.json 和 slice_metadata.json，同时核对每层文件。额外留空与体素覆盖使外接栅格尺寸大于模型几何尺寸。

原始姿态可能增加 X 宽度。128 喷孔设备可显式设置 output_width_px=128；超宽时作业会报错。选用保持尺寸时可设置 target_height_mm=0，不能与最长边同时指定。