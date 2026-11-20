"""Run from any directory: python platform_smoke.py /path/to/bin/fry-halftone-cli"""
import argparse
import hashlib
import json
import subprocess
import tempfile
from pathlib import Path

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def execute(cli, job_file, request):
    job_file.write_text(json.dumps(request, ensure_ascii=False), encoding="utf-8")
    return subprocess.run([str(cli), "run", str(job_file), "--json"], capture_output=True,
                          text=True, encoding="utf-8", errors="replace", timeout=180)

def main(cli):
    data = Path(__file__).resolve().parent
    expected = json.loads((data / "expected.json").read_text(encoding="utf-8"))
    model = data / "vertex_gray_box.glb"
    if digest(model) != expected["model_sha256"]:
        raise ValueError("测试模型校验失败")
    info = json.loads(subprocess.check_output([str(cli), "info", "--json"], encoding="utf-8"))
    if info["version"] != "0.4.0" or info["algorithmFingerprint"] != expected["algorithm_fingerprint"]:
        raise ValueError("SDK 身份不匹配")
    with tempfile.TemporaryDirectory(prefix="fry_sdk_smoke_") as tmp:
        work = Path(tmp)
        output = work / "slices"
        job = json.loads((data / "request.json").read_text(encoding="utf-8"))
        job.update(model=str(model), out=str(output))
        result = execute(cli, work / "request.json", job)
        if result.returncode:
            raise RuntimeError((result.stdout + result.stderr)[-2000:])
        events = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
        results = [e["result"] for e in events if e.get("type") == "result"]
        if len(results) != 1 or not results[0]["success"]:
            raise ValueError("缺少成功结果")
        raw = hashlib.md5()
        count = expected["slice_count_int"]
        names = sorted(output.glob("*.bmp"))
        if [p.name for p in names] != [f"{i:06d}.bmp" for i in range(count)]:
            raise ValueError("BMP 层号或张数不符")
        for path in names:
            raw.update(path.read_bytes())
        sdk = results[0]["sdkResult"]
        md = json.loads((output / "slice_metadata.json").read_text(encoding="utf-8"))
        if (raw.hexdigest() != expected["bmp_md5_str"] or sdk["sliceCount"] != count
                or sdk["inkedVoxels"] != expected["inked_voxel_int"] or md["band_thickness_mm_float"] != expected["band_thickness_mm_float"]
                or sdk["algorithm"] != "water_transfer_unified_he520"):
            raise ValueError("模型出片数据不符")
        job["out"] = str(work / "invalid")
        job["params"]["wt_preset"] = "he510"
        job["params"]["wt_outward_strength"] = 0.5
        invalid = execute(cli, work / "invalid.json", job)
        if invalid.returncode == 0 or list((work / "invalid").glob("*.bmp")):
            raise ValueError("无效组合被接受")
        return {"version":info["version"], "he520_slices":count, "md5":raw.hexdigest(),
                "invalid_request_cli_exit":invalid.returncode, "ok":True}

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("cli", type=Path)
    print(json.dumps(main(parser.parse_args().cli.resolve()), indent=2))
