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
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        work = Path(tmp)
        request = json.loads((root / "examples/original.json").read_text(encoding="utf8"))
        request.update(model=str(root / request["model"]), out=str(work / "bmp"))
        job = work / "original.json"
        job.write_text(json.dumps(request), encoding="utf8")
        subprocess.run([str(tool),"run",str(job)],check=True,timeout=180)
        out = Path(request["out"])
        matrix = json.loads((out / "_orient.json").read_text(encoding="utf8"))["glb_to_voxel_matrix"]
        layers = sorted(out.glob("*.bmp"))
        digest = hashlib.md5()
        for layer in layers:
            digest.update(layer.read_bytes())
        if matrix != [[1,0,0],[0,1,0],[0,0,1]] or len(layers)!=309 or digest.hexdigest()!=contract["original_bmp_md5"]:
            raise ValueError("Original orientation golden mismatch")
    print(json.dumps({"product_version":contract["product_version"], "sdk_version":contract["runtime_version"],
                      "target":target, "passed":True}))
if __name__ == "__main__":
    main()
