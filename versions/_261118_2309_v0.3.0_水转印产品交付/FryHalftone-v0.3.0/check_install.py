"""Product acceptance. Run with Python 3.9+ from any directory."""
import hashlib
import json
from pathlib import Path
import platform
import subprocess
import sys

def host_target():
    system = {"Windows": "win32", "Linux": "linux", "Darwin": "darwin"}
    arch = {"amd64": "x64", "x86_64": "x64", "arm64": "arm64", "aarch64": "arm64"}
    return system.get(platform.system(), "unknown") + "-" + arch.get(platform.machine().lower(), "unknown")

def main():
    root = Path(__file__).resolve().parent
    contract = json.loads((root / "PRODUCT.json").read_text(encoding="utf8"))
    target = host_target()
    if target not in contract["platforms"]:
        raise ValueError("Unsupported worker OS/architecture: " + target)
    entry = contract["platforms"][target]
    for key in ("library", "cli"):
        path = root / entry[key]
        actual = hashlib.sha256(path.read_bytes()).hexdigest()
        if actual != entry[key + "_sha256"]:
            raise ValueError("SHA256 mismatch: " + entry[key])
    subprocess.run([sys.executable, "-X", "utf8", str(root / entry["package_dir"] / "verify_delivery.py")],
                   check=True, timeout=120)
    tool = root / entry["cli"]
    actual_schema = json.loads(subprocess.check_output([str(tool), "schema"], encoding="utf8", timeout=20))
    if actual_schema != json.loads((root / contract["parameter_schema"]).read_text(encoding="utf8")):
        raise ValueError("Parameter schema mismatch")
    subprocess.run([sys.executable, "-X", "utf8", str(root / "examples/platform_smoke.py"), str(tool)],
                   check=True, timeout=240)
    print(json.dumps({"product_version":contract["product_version"], "sdk_version":contract["runtime_version"],
                      "target":target, "passed":True}))
if __name__ == "__main__":
    main()
