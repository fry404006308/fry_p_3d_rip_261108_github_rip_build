# -*- coding: utf-8 -*-
r"""verify_delivery.py —— ★393 交付包自检（随包发给对接方，只用 Python 标准库）。

【给拿到这个包的人】
  双击或者命令行跑一次：
      python verify_delivery.py
  它会回答三个问题：
      ① 这个包**完整吗**            —— 按包里的 delivery_manifest.json 逐项核对（缺件 / 空文件 / 内容被改），
                                       ★411 再按 SHA256SUMS.txt 逐行核对一遍（= `sha256sum -c`；两把尺子互相独立）
      ② 这个 dll **是哪一版**       —— 版本号在四处都读一遍，必须一致
      ③ 这个 dll **能加载吗**       —— 真加载一次、真调一次 GetVersion()（导出符号、运行库依赖都会在这一步现形）
  全绿 = 可以直接集成；有红 = 把输出整段发回给我们，不用再描述现象。

【为什么要随包给自检脚本】把「包对不对」这件事变成对方**自己能验证**的，
  比任何说明文字都省事 —— 也省掉「你确认一下是不是发漏了」这类来回。
"""
from __future__ import annotations

import ctypes
import hashlib
import json
import platform
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
LIB_NAME_DICT = {"Windows": "fry_halftone.dll", "Linux": "libfry_halftone.so", "Darwin": "libfry_halftone.dylib"}


def sha256_file(p: Path) -> str:
    h = hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def find_library() -> Path:
    name = LIB_NAME_DICT.get(platform.system(), "")
    for cand in (HERE / "bin" / name, HERE / name):
        if cand.is_file():
            return cand
    return Path()


def read_fileinfo_version(lib: Path) -> str:
    """Windows「右键 → 属性 → 详细信息」里的版本号。"""
    if platform.system() != "Windows" or not lib.is_file():
        return ""
    try:
        ver = ctypes.WinDLL("version")
        size = ver.GetFileVersionInfoSizeW(ctypes.c_wchar_p(str(lib)), None)
        if not size:
            return ""
        buf = ctypes.create_string_buffer(size)
        ver.GetFileVersionInfoW(ctypes.c_wchar_p(str(lib)), 0, size, buf)
        block, length = ctypes.c_void_p(), ctypes.c_uint()
        if ver.VerQueryValueW(buf, ctypes.c_wchar_p(r"\VarFileInfo\Translation"), ctypes.byref(block), ctypes.byref(length)) and length.value >= 4:
            raw = ctypes.string_at(block.value, 4)
            key = r"\StringFileInfo\%04x%04x\FileVersion" % (int.from_bytes(raw[:2], "little"), int.from_bytes(raw[2:], "little"))
            if ver.VerQueryValueW(buf, ctypes.c_wchar_p(key), ctypes.byref(block), ctypes.byref(length)) and length.value:
                return ctypes.wstring_at(block.value, length.value).strip("\x00").strip()
    except Exception as e:                                              # noqa: BLE001
        return "<读失败: %s>" % e
    return ""


def load_and_ask(lib: Path) -> dict:
    """真加载一次：这一步顺带验证了导出符号齐全、运行库依赖都在。"""
    out = {}
    try:
        dll = ctypes.WinDLL(str(lib)) if platform.system() == "Windows" else ctypes.CDLL(str(lib))
    except OSError as e:
        return {"error_str": "加载失败: %s" % e}
    for fn, key in (("FryHalftone_GetVersion", "version_str"),
                    ("FryHalftone_GetAlgorithmFingerprint", "fingerprint_str"),
                    ("FryHalftone_GetAlgorithmName", "algorithm_str"),
                    ("FryHalftone_GetEngine", "engine_str")):
        try:
            f = getattr(dll, fn)
            f.restype = ctypes.c_char_p
            out[key] = (f() or b"").decode("utf-8", "replace")
        except AttributeError:
            out["error_str"] = "库里没有导出 %s（这不是本 SDK 的库？）" % fn
    try:
        f = dll.FryHalftone_GetParamSchemaJson
        f.restype = ctypes.c_char_p
        out["param_count_int"] = len(json.loads((f() or b"{}").decode("utf-8")).get("params", []))
    except Exception:                                                   # noqa: BLE001
        pass
    return out


def main() -> int:
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    except Exception:                                                   # noqa: BLE001
        pass
    print("=" * 76)
    print("FryHalftone SDK 交付包自检　（目录：%s）" % HERE)
    print("=" * 76)
    mpath = HERE / "delivery_manifest.json"
    if not mpath.is_file():
        print("✗ 这个目录里没有 delivery_manifest.json —— 不是完整的交付包，请向我们索要完整包。")
        return 1
    man = json.loads(mpath.read_text(encoding="utf-8"))
    print("包名 %s ｜ 平台 %s ｜ 等级 %s ｜ 清单版本 %s" %
          (man.get("package_str", "?"), man.get("platform_str", "?"), man.get("level_str", "?"), man.get("schema_str", "?")))

    bad = []
    rows = man.get("files_list") or []
    for row in rows:
        rel = str(row.get("path_str", ""))
        p = HERE / rel
        if not p.is_file():
            print("  ✗ 缺 %-34s （%s：%s）" % (rel, row.get("cat_str", ""), row.get("why_str", "")))
            bad.append("缺 " + rel)
            continue
        want = str(row.get("sha256_str") or "")
        got = sha256_file(p) if want else ""
        if want and got != want:
            print("  ✗ %-34s 内容与清单不符（被改过或传输损坏）" % rel)
            bad.append("内容不符 " + rel)
        else:
            print("  ✓ %-34s %8d B" % (rel, p.stat().st_size))

    # ★411 第二把尺子：SHA256SUMS.txt 逐行核对（410 之前出包时它里面清单那一行一直是过期的，只看清单的自检发现不了）
    sums = HERE / "SHA256SUMS.txt"
    if sums.is_file():
        n_all, n_bad = 0, 0
        for line in sums.read_text(encoding="utf-8").splitlines():
            if not line.strip():
                continue
            want, _, rel = line.partition("  ")
            p = HERE / rel
            n_all += 1
            if not p.is_file() or sha256_file(p) != want:
                n_bad += 1
                print("  ✗ SHA256SUMS.txt 里的 %s %s" % (rel, "不存在" if not p.is_file() else "内容不符（被改过或传输损坏）"))
                bad.append("SHA256SUMS 不符 " + rel)
        if n_all == 0:
            bad.append("SHA256SUMS.txt 是空的")
        print("  %s SHA256SUMS.txt：核对 %d 个文件，%d 个不符" % ("✓" if n_all and not n_bad else "✗", n_all, n_bad))
    else:
        print("  · 本目录没有 SHA256SUMS.txt（装机目录只带运行件，这一项跳过）")

    lib = find_library()
    print("-" * 76)
    if not lib.is_file():
        print("  ✗ 没找到动态库本体")
        bad.append("没有动态库")
    else:
        ident = load_and_ask(lib)
        if ident.get("error_str"):
            print("  ✗ %s" % ident["error_str"])
            bad.append(ident["error_str"])
        else:
            print("  ✓ 加载成功：版本 %s ｜ 算法 %s ｜ 引擎 %s ｜ 参数 %s 个"
                  % (ident.get("version_str"), ident.get("algorithm_str"), ident.get("engine_str"), ident.get("param_count_int", "?")))
            print("    算法指纹 %s" % ident.get("fingerprint_str", ""))
        info = {}
        for cand in (HERE / "sdk_info.json",):
            if cand.is_file():
                info = json.loads(cand.read_text(encoding="utf-8"))
        vers = {"VERSION 文件": (HERE / "VERSION").read_text(encoding="utf-8").strip() if (HERE / "VERSION").is_file() else "",
                "sdk_info.json": str(info.get("version_str", "")),
                "dll 自报": ident.get("version_str", "")}
        if platform.system() == "Windows":           # 只有 Windows 的 dll 有「文件属性」这一处（Linux / macOS 没有，不算缺）
            vers["文件属性"] = read_fileinfo_version(lib)
        shown = {k: v for k, v in vers.items() if v}
        print("  版本号：" + " ｜ ".join("%s=%s" % (k, v) for k, v in vers.items()))
        if len(set(shown.values())) > 1:
            print("  ✗ 版本号不一致 —— 这个包是拼起来的，别用")
            bad.append("版本号不一致")
        missing = [k for k, v in vers.items() if not v]
        if missing:
            print("  ⚠ 这几处读不到版本号：%s" % "、".join(missing))

    print("=" * 76)
    if bad:
        print("✗ 自检没过（%d 条）：请把上面整段输出发回给我们。" % len(bad))
        return 1
    print("✓ 自检通过：包完整、dll 能加载、版本号处处一致 —— 可以直接集成。")
    print("  下一步：看 SDK_使用说明.md（集成说明）与 examples/（C / C# / Python 三个示例）。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
