# -*- coding: utf-8 -*-
r"""fry_halftone_demo.py —— ★393 Python(ctypes) 调用示例：不装任何第三方包就能跑。

【怎么跑】
  python fry_halftone_demo.py <模型.glb> <输出目录> [dll 路径]
  不给 dll 路径时，按这个顺序找：同目录 → ../bin/ → 环境变量 FRY_HALFTONE_LIB。

【Python 对接这个库的三个要点】
  ① Windows 上必须用 ctypes.WinDLL（__stdcall）；Linux / macOS 用 ctypes.CDLL。
  ② 走 …Utf8 那一组接口：传 bytes（UTF-8），不要传 str。
  ③ 进度回调要用 CFUNCTYPE / WINFUNCTYPE 包一层，并且**保住引用**（存到变量里），否则会被回收。
"""
from __future__ import annotations

import ctypes
import json
import os
import platform
import sys
from pathlib import Path

LIB_NAME_DICT = {"Windows": "fry_halftone.dll", "Linux": "libfry_halftone.so", "Darwin": "libfry_halftone.dylib"}


def find_library(hint: str = "") -> Path:
    name = LIB_NAME_DICT[platform.system()]
    here = Path(__file__).resolve().parent
    for cand in (Path(hint) if hint else None, here / name, here.parent.parent / "bin" / name,
                 Path(os.environ.get("FRY_HALFTONE_LIB", "")) if os.environ.get("FRY_HALFTONE_LIB") else None):
        if cand is None:
            continue
        p = Path(cand)
        if p.is_dir():
            p = p / name
        if p.is_file():
            return p
    raise SystemExit("✗ 找不到 %s —— 用第三个参数指定它的完整路径" % name)


class FryHalftone:
    def __init__(self, path: Path) -> None:
        is_win = platform.system() == "Windows"
        self.lib = ctypes.WinDLL(str(path)) if is_win else ctypes.CDLL(str(path))      # ① 调用约定
        cb = ctypes.WINFUNCTYPE if is_win else ctypes.CFUNCTYPE                        # ③ 回调类型
        self.PROGRESS = cb(None, ctypes.c_int, ctypes.c_char_p, ctypes.c_void_p)
        for fn in ("FryHalftone_GetVersion", "FryHalftone_GetAlgorithmFingerprint",
                   "FryHalftone_GetAlgorithmName", "FryHalftone_GetEngine", "FryHalftone_GetParamSchemaJson"):
            getattr(self.lib, fn).restype = ctypes.c_char_p
            getattr(self.lib, fn).argtypes = []
        self.lib.FryHalftone_SliceModelJsonUtf8.restype = ctypes.c_int
        self.lib.FryHalftone_SliceModelJsonUtf8.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int,
                                                            self.PROGRESS, ctypes.c_void_p,
                                                            ctypes.c_char_p, ctypes.c_int]
        self.lib.FryHalftone_Cancel.restype = ctypes.c_int
        self.lib.FryHalftone_Cancel.argtypes = []
        self._keep = None                                                              # ③ 保住回调引用

    def version(self) -> str:
        return self.lib.FryHalftone_GetVersion().decode("utf-8")

    def fingerprint(self) -> str:
        return self.lib.FryHalftone_GetAlgorithmFingerprint().decode("utf-8")

    def param_schema(self) -> dict:
        return json.loads(self.lib.FryHalftone_GetParamSchemaJson().decode("utf-8"))

    def slice_model(self, request: dict, on_progress=None) -> tuple:
        def _cb(percent, stage, _user):
            if on_progress:
                on_progress(int(percent), (stage or b"").decode("utf-8", "replace"))
        self._keep = self.PROGRESS(_cb)
        result = ctypes.create_string_buffer(8192)                                     # ⚠ 不够会返回 2 并写明需要多少
        err = ctypes.create_string_buffer(2048)
        rc = self.lib.FryHalftone_SliceModelJsonUtf8(json.dumps(request, ensure_ascii=False).encode("utf-8"),
                                                     result, len(result), self._keep, None, err, len(err))
        return rc, result.value.decode("utf-8", "replace"), err.value.decode("utf-8", "replace")


def main(argv) -> int:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    if len(argv) < 3:
        print(__doc__)
        return 2
    sdk = FryHalftone(find_library(argv[3] if len(argv) > 3 else ""))
    print("SDK 版本  :", sdk.version())
    print("算法指纹  :", sdk.fingerprint())
    print("参数个数  :", len(sdk.param_schema()["params"]))
    request = {"model": argv[1].replace("\\", "/"), "out": argv[2].replace("\\", "/"),
               "params": {"target_longest_edge_mm": 16.851,
                          "ink_c_um": 400, "ink_m_um": 500, "ink_y_um": 300,
                          "band_uninked_fill": "transparent", "support_enable": True,
                          "orient_mode": "auto_extent"}}
    rc, result, err = sdk.slice_model(request, on_progress=lambda p, s: print("  %3d%%  %s" % (p, s)))
    if rc != 0:
        print("✗ 失败，返回码 %d：%s" % (rc, err))          # 7 = 模型用到了本库暂不支持的写法，原因在 err 里
        return rc
    info = json.loads(result)
    print("✓ 成功：%d 张切片 → %s" % (info.get("sliceCount", -1), info.get("outputDirectory", "")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
