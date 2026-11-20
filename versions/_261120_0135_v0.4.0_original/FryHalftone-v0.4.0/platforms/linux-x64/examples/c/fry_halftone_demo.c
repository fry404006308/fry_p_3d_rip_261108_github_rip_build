/*
 * fry_halftone_demo.c —— 给桌面端同事的**最小可编译示例**（纯 C，不是 C++）。
 *
 * 为什么是纯 C：fry_halftone.h 声称「纯 C 也能包含」。这句话不编一次就是一句没验过的承诺
 * （extern "C" 在 C 编译器里是语法错误，写错了 C++ 消费方毫无察觉，C 消费方直接编不过）。
 * CMake 把本文件编成 FryHalftoneDemoC，出包冒烟时还会真跑一次 —— 承诺变成了判据。
 *
 * ⚠ Windows 上命令行参数必须按**宽字符**取（wmain）再调宽字符版接口：main 的 argv 是本地代码页，
 *   中文路径会变成问号 ⇒「找不到模型文件」（384 实测踩过：D:/_300_3D_Rip/_260820_??/…）。
 *
 * 用法：fry_halftone_demo <model.glb> <输出目录> [成品 z 高度mm（与 lmst targetHeightMm 同义），默认 0 = 保持模型尺寸]
 * 编译 + 运行（★411 在**解包后的 fry-halftone/ 目录里**执行；Windows / Linux 两组逐条实测过，唯一真源：打包仓 tools/fry_integration_guide.py）：
 *   Windows（x64 Native Tools Command Prompt for VS）:
 *     cl /nologo /utf-8 /I include examples\c\fry_halftone_demo.c lib\fry_halftone.lib /Fo:bin\ /Fe:bin\fry_halftone_demo.exe
 *     bin\fry_halftone_demo.exe <模型.glb> out_c 1.6
 *   Linux:
 *     cc examples/c/fry_halftone_demo.c -Iinclude -Lbin -lfry_halftone -Wl,-rpath,'$ORIGIN' -o bin/fry_halftone_demo
 *     bin/fry_halftone_demo <模型.glb> out_c 1.6
 *   macOS（Mac 上验收通过之后；未在 Mac 上逐条实测）:
 *     cc examples/c/fry_halftone_demo.c -Iinclude -Lbin -lfry_halftone -o bin/fry_halftone_demo
 *     bin/fry_halftone_demo <模型.glb> out_c 1.6
 *   ⚠ 可执行文件要和库在同一个目录（上面都输出到 bin/）：Windows 就近加载 dll；Linux 的 RUNPATH=$ORIGIN、
 *     macOS 库的 install_name=@loader_path/ 都只认同目录。★411 以前这里写的是 `bin\fry_halftone.lib`（导入库其实在 lib\，
 *     实测 LNK1181）与不带 -o 的 cc（a.out 落在包根，实测运行时报「cannot open shared object file」）。
 */
#include <stdio.h>
#include <stdlib.h>

#include "fry_halftone.h"

#ifdef _WIN32
#include <windows.h>

/* 进度回调：在调用 SliceModel 的那个线程上同步触发（见头文件 ④） */
static void FRY_CALL on_progress_w(int percent, const wchar_t* stage, void* user_data) {
    char buf[512];
    (void)user_data;
    buf[0] = '\0';
    if (stage) WideCharToMultiByte(CP_UTF8, 0, stage, -1, buf, (int)sizeof(buf), NULL, NULL);
    printf("[%3d%%] %s\n", percent, buf);
    fflush(stdout);
}

int wmain(int argc, wchar_t** argv) {
    wchar_t err[4096] = {0};
    char err8[8192] = {0};
    double target_mm = 0.0;
    int rc;
    SetConsoleOutputCP(CP_UTF8);
    if (argc < 3) {
        fprintf(stderr, "usage: FryHalftoneDemoC <model.glb> <out_dir> [z_height_mm]\n");
        return 2;
    }
    if (argc >= 4) target_mm = _wtof(argv[3]);
    printf("SDK 版本   : %s\n", FryHalftone_GetVersion());
    printf("加网算法   : %s（引擎 %s）\n", FryHalftone_GetAlgorithmName(), FryHalftone_GetEngine());
    printf("算法指纹   : %s\n", FryHalftone_GetAlgorithmFingerprint());
    rc = FryHalftone_SliceModel(argv[1], argv[2], target_mm, on_progress_w, NULL, err, (int)(sizeof(err) / sizeof(err[0])));
    if (rc != FRY_HALFTONE_OK) {
        WideCharToMultiByte(CP_UTF8, 0, err, -1, err8, (int)sizeof(err8), NULL, NULL);
        fprintf(stderr, "失败（返回码 %d）：%s\n", rc, err8);
        return 1;
    }
    printf("成功：切片与 slice_metadata.json 已写出\n");
    return 0;
}
#else
static void FRY_CALL on_progress(int percent, const char* stage, void* user_data) {
    (void)user_data;
    printf("[%3d%%] %s\n", percent, stage ? stage : "");
    fflush(stdout);
}

int main(int argc, char** argv) {
    char err[4096] = {0};
    double target_mm = 0.0;
    int rc;
    if (argc < 3) {
        fprintf(stderr, "usage: FryHalftoneDemoC <model.glb> <out_dir> [z_height_mm]\n");
        return 2;
    }
    if (argc >= 4) target_mm = atof(argv[3]);
    printf("SDK 版本   : %s\n", FryHalftone_GetVersion());
    printf("加网算法   : %s（引擎 %s）\n", FryHalftone_GetAlgorithmName(), FryHalftone_GetEngine());
    printf("算法指纹   : %s\n", FryHalftone_GetAlgorithmFingerprint());
    rc = FryHalftone_SliceModelUtf8(argv[1], argv[2], target_mm, on_progress, NULL, err, (int)sizeof(err));
    if (rc != FRY_HALFTONE_OK) {
        fprintf(stderr, "失败（返回码 %d）：%s\n", rc, err);
        return 1;
    }
    printf("成功：切片与 slice_metadata.json 已写出\n");
    return 0;
}
#endif
