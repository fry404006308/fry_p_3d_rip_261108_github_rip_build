# 五平台原生验收

[本次 CI](https://github.com/fry404006308/fry_p_3d_rip_261108_github_rip_build/actions/runs/44712043648)。新收录的 original_world_axes_l3 使用独立 Python 参考结果；同一组 24 个作业通过动态库和 CLI 两次出片比对。

|目标|真实系统|库|CLI|CLI 契约|
|---|---|---:|---:|---:|
|win32-x64|Windows-10-10.0.20348-SP0|24/24|24/24|45|
|darwin-arm64|macOS-15.8.2-arm64-arm-64bit|24/24|24/24|45|
|darwin-x64|macOS-15.7.9-x86_64-i386-64bit|24/24|24/24|45|
|linux-arm64|Linux-7.0.0-1017-azure-aarch64-with-glibc2.39|24/24|24/24|45|
|linux-x64|Linux-6.8.0-1070-azure-x86_64-with-glibc2.35|24/24|24/24|45|

新增用例中身份矩阵保留了节点旋转后的场景 Z。带尺寸、自动与指定方向、墨水、外移参数的原有 23 个参考用例仍全部通过。
