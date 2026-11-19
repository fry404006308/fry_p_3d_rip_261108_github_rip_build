# FryHalftone SDK — quick reference (English)

Water-transfer 3D halftone (`water_transfer_depth`): **one coloured .glb in → a stack of 4-bit palette BMP slices out**.
C ABI in `include/fry_halftone.h`; the call shape mirrors lmst's `LmstYzHalftone_*` so existing desktop code ports by renaming.

## Start here (★393)

```
python verify_delivery.py
```

Answers three questions in one run: is the package **complete** (checked item by item against `delivery_manifest.json`,
including each file's sha256), does the library **load**, and is the version **the same in all four places**
(`VERSION`, `sdk_info.json`, `FryHalftone_GetVersion()`, and the Windows file properties). Green means you can integrate.

Integration facts you would otherwise have to ask for are all in **`sdk_info.json`**: `abi_str` (C ABI, `__stdcall` on Windows),
`runtime_str` (C++ runtime statically linked — **no VC++ Redistributable needed**), `min_os_str`, `thread_safety_str`,
`memory_hint_str`, `entry_points_list`, `return_codes_dict`, `file_properties_dict`.
Examples for three languages live in `examples/` (C, C#, Python); the licence and third-party notice is `LICENSE.txt`
(this library links **no** third-party open-source code).

## Must know

1. **Since 384 the algorithm is C++ and runs in your process** — ship only `bin/fry_halftone.dll` (`.so` / `.dylib`).
   Output is **byte-identical** to the algorithm repository's Python production line
   (21 models × 23 intermediate arrays + every BMP; since ★391 **10** golden cases re-checked on every package build,
   including a vertex-coloured model with a white material and a "colour-mode sampler" model with 23 glTF colouring variants).
   The reference hemisphere (16.851 mm) takes 3.6 s and 0.83 GB peak memory (Python on the same machine: 102 s, 11.5 GB).
2. One job at a time per loaded library; a concurrent call returns `FRY_HALFTONE_BUSY (6)`.
3. The progress callback fires on the calling thread; call `FryHalftone_Cancel()` from another thread
   (the job stops at the next checkpoint and returns `FRY_HALFTONE_CANCELLED (5)`).
4. ★391 **Colour follows the glTF spec**: base colour = `baseColorFactor × texture × COLOR_0` (a missing term counts as 1; a material
   without `baseColorFactor` is white), `TEXCOORD_n` + `KHR_texture_transform`, sampler wrapS/wrapT (repeat / clamp / mirror) and
   `magFilter` NEAREST, specular-glossiness diffuse, float / u8 / u16 vertex colours; triangle strips and fans, sparse accessors,
   quantized positions and UVs (`KHR_mesh_quantization`), data-URI and sibling-file images are supported.
   Unsupported model features return `FRY_HALFTONE_UNSUPPORTED (7)` with the reason — never a silently different result:
   progressive / 12-bit / CMYK JPEG, WebP / KTX2 / AVIF / DDS without a PNG/JPEG fallback, Draco / meshopt compression,
   GPU instancing, unknown *required* extensions, GLB buffers with a uri, `.gltf` text files,
   a node with several parents, and near-rigid node matrices (trimesh repairs them with an SVD).
   Since ★390 the voxelizer uses the same `exact` route on both sides (bit-identical), so **large parts are no longer refused**;
   size is limited only by memory, which grows linearly with the voxel count.
   Corrupt files (truncated, out-of-range offsets, NaN or overflowing coordinates) return `FRY_HALFTONE_FAILED (1)`; fuzzed with 174 000 broken files
   (the first round found and fixed one infinite loop; the second round had no crash and no hang).

## Minimal call

```c
char err[4096];
int rc = FryHalftone_SliceModelUtf8("a.glb", "out_dir", 30.0 /* Z height mm, like lmst */, on_progress, NULL, err, sizeof(err));
```

Return codes: 0 OK · 1 failed · 2 bad argument · 5 cancelled · 6 busy · 7 unsupported input (3 / 4 were subprocess-era codes and are never returned now).

## JSON API (preferred for new code — adding parameters never changes the ABI)

`FryHalftone_SliceModelJsonUtf8({"model": …, "out": …, "params": {…}}, resultJson, 8192, cb, user, err, 4096)`;
`FryHalftone_GetParamSchemaJson()` returns the whitelist below as JSON. Since 387 (`fry-halftone-params/3`) every entry also carries
`default` (the value used when the key is omitted), `nullable` and its range (`min` / `min_exclusive` / `max` / `integer` / `choices` / `length`),
plus top-level `one_of_required` and `requires` rules. The package ships the same JSON as **`params_schema.json`** and a generated table
**`SDK_参数说明.md`**; both are exported from the library itself at packaging time, and the package smoke test runs the library to confirm
that omitting a key equals passing the documented default (identical BMP md5) and that 75 boundary values are accepted or rejected exactly as documented.
**Treat those two files as authoritative** — the table below is the annotated version.

| Parameter | JSON type | Default | Meaning |
| --- | --- | --- | --- |
| `target_height_mm` | number ≥ 0 or null | — | **Z height** after box-axis orientation (shortest extent → Z); same meaning as lmst's `targetHeightMm`; 0 = keep authored size |
| `target_longest_edge_mm` | number > 0 or null | — | longest edge (the algorithm repository's `TARGET_MM`). **Exactly one of the two size keys is required** |
| `ink_c_um` / `ink_m_um` / `ink_y_um` | number ≥ 0 | 400 / 500 / 300 | full-coverage ink thickness in µm |
| `use_black_k` | boolean | false | CMYK: the grey component min(C,M,Y) × `black_k_amount` moves to K ink (reserved for a future K ink) |
| `ink_k_um` | number ≥ 0 or null | null | K full-coverage thickness; **required** when `use_black_k` is true |
| `black_k_amount` | number in [0, 1] | 1.0 | grey-component replacement strength |
| `color_thickness_mm` | number > 0 or null | null | colour-band thickness; null = sum of the full-coverage thicknesses (C+M+Y[+K]). Thinner than the part's ink demand truncates ink (lighter, hue-shifted); thicker spreads ink deeper (frontal colour barely changes, sharpness drops when very thick). Not a lightness control |
| `band_uninked_fill` | `"transparent"` \| `"white"` | transparent | what fills band cells that receive no colour ink |
| `support_enable` | **JSON boolean** | true | print support (the string `"false"` is rejected) |
| `output_width_px` | positive integer or null | null | BMP width (nozzles); null = model width |
| `voxel_mm` | 3 positive numbers | [0.137, 0.035, 0.01] | droplet size x, y, z (z = layer height) |
| `threads` | non-negative integer or null | null | worker threads; null / 0 = all hardware threads |
| ★449 `wt_engine` | `"legacy"` \| `"unified"` | legacy | water-transfer halftone engine; legacy = original HE500 (**omitting it is byte-identical to the previous release**); unified = one code for HE500 / HE510 / HE520 |
| ★449 `wt_preset` | `"he500"` \| `"he510"` \| `"he520"` \| `"custom"` | he500 | **only with `unified`**: he500 = no outward move (= legacy byte for byte), he510 = normal columns + full outward move, he520 = optical move + blue-noise rounding dither, custom = the four switches below |
| ★449 `wt_outward_columns` | `"normal"` \| `"edt"` | normal | only for he510 / he520: rebuild columns along normals (default) or keep the EDT columns |
| ★449 `wt_outward_strength` / `wt_outward_attenuation_mm_inv` | number in [0, 1] / number > 0 | 0.5 / 4.0 | only for he520 / custom: optical move strength a and attenuation λ (1/mm) |
| ★449 `wtu_columns` / `wtu_ray_origin` / `wtu_move` / `wtu_dither` | string | normal / voxel / optical / blue_noise | only for custom; the defaults equal he520 |

**Unknown keys or wrong types are rejected (rc 2) with a message naming the parameter — never silently ignored.**
★449 Keys that would have no effect (e.g. `wt_preset` with `legacy`) are rejected the same way (rules: `only_when` in `params_schema.json`);
with `legacy` the nine `wt_*` keys are not echoed in `slice_metadata.json` (rule `echo_omit_when`), so existing callers see the same echo as before.
★449 ⚠ Normal columns (he510 / he520 / custom normal) combined with an `orient_yaw_deg` that is not a multiple of 90° follow the last bit of the platform's `sin`/`cos`:
each platform's library equals the Python reference on the same platform, but Windows and Linux / macOS may differ by a few voxels (same inked count).
Legacy, or yaw in multiples of 90°, is byte-identical on all three platforms.

The result JSON carries `sliceCount`, `inkedVoxels`, `bmpMd5`, `bandThicknessMm`, `colorChannels`, `params`, `size`,
`engine` (`"cpp"`), `algorithmFingerprint` and `elapsedSeconds`; `slice_metadata.json` in the output folder records the same.

## ★411 Output: BMP pixel value (palette index) → material

Each pixel of the 4-bit BMP stores a **palette index**; the 16-entry palette itself is greyscale (odd indices = grey ramp, even = black),
so **identify materials by pixel value, not by colour**. Single source: `kPal` in `cpp/src/output/assemble.cpp` (a packaging test checks every row).

| Pixel value | Material | Jetted? | When it appears |
| ---: | --- | --- | --- |
| 0 | Null (empty) | no | outside the model / nothing to print |
| 1 | C (cyan) | yes | colour ink |
| 3 | M (magenta) | yes | colour ink |
| 5 | Y (yellow) | yes | colour ink |
| 7 | K (black) | yes | only with `use_black_k=true` |
| 9 | White | yes | white material (base); also un-inked band cells when `band_uninked_fill="white"` |
| 11 | Transparent | yes | clear shell; also un-inked band cells when `band_uninked_fill="transparent"` (default) |
| 13 | Support | yes | when `support_enable=true` (default) |

Other values (2, 4, 6, 8, 10, 12, 14, 15) never occur. Only 0 means "print nothing" — White / Transparent / Support are all jetted.
Per-material voxel totals: `material_count_dict` in `slice_metadata.json`.

## Identity

`FryHalftone_GetAlgorithmFingerprint()` returns the sha256 of the exact C++ source tree compiled into the library
(line endings normalised, so Windows and Linux checkouts agree) — file names can be wrong, the fingerprint cannot.
`sdk_info.json` records `library_sha256_str` (since 387): hash `bin/` yourself and compare to detect a library swapped without its package metadata.

## ★412 Version numbers (decided automatically at packaging time)

Delivered builds are always `X.Y.Z`, increasing from 0.1.0 (`0.0.0-dev` = local debug build, never delivered).
Which part goes up follows [SemVer](https://semver.org/), comparing this delivery with the previous one on the **public surface**
(C function signatures, parameter names + types, CLI subcommands + options) and the **algorithm fingerprint**:
removed/changed item → major · added item, or algorithm fingerprint changed (same input ⇒ different slices, re-validate) → minor ·
implementation/docs/packaging only → patch. While the major is 0 (surface not yet promised stable) a breaking change bumps the
minor only; 1.0.0 is a human decision (after the Mac verification passes). ⇒ **patch only = byte-identical output for the same input**.
The reason for each bump is in `CHANGELOG.md` and in section 1 of the delivery README.

## Platforms

Windows x64 (no VC++ redistributable needed; the C math library is the system `ucrtbase.dll`, the same one numpy uses) ·
Linux x64 (built on Debian bookworm, glibc ≥ 2.36, libstdc++ linked statically) ·
macOS arm64 (built with `-ffp-contract=off`; ad-hoc signed, not notarized: run `xattr -dr com.apple.quarantine <dir>` after downloading).
⚠ Only the Windows build has been byte-compared with Python so far; the golden test runs in every platform's CI job.

## ★409 Command line: `fry-halftone-cli`

Every platform package ships `bin/fry-halftone-cli` (`.exe` on Windows) next to the library. It only calls the 11 public C
functions and links the **same** shared library, so CLI slices are byte-identical to library slices (the build re-runs all
13 golden cases through the CLI). The protocol mirrors lmst's `lmst-cli`:

```
fry-halftone-cli slice <model.glb> --out <dir> (--target-height <mm> | --target-longest-edge <mm>)
                 [--layer-height <mm>] [--no-auto-orient] [--threads <n>] [--params <file.json>] [--json]
fry-halftone-cli run <request.json> [--json] | schema | info [--json] | --version | --help
```

- stderr: `[fry-halftone-cli] progress=N stage=...`, last line on success `[fry-halftone-cli] done. slices=N`
- `--json`: one JSON event per stdout line (progress events + one `result` envelope)
- the output directory must be empty; a key given both as a flag and in `--params` is rejected (exit 2)
- exit codes: 0 ok | 2 usage/bad params | 3 input missing | 4 output dir | 11 slice failed | 12 incomplete output

## ★409 Minimum OS (measured from the binaries, not copied from build settings)

| Package | Loads on | Byte-identical golden verified on |
| --- | --- | --- |
| win32-x64 | Windows 10 x64+ | Windows 11 (build machine) |
| linux-x64, `prebuilt` (zig, glibc 2.28 baseline) | glibc ≥ 2.17 (highest `GLIBC_x.y` symbol version referenced) | Debian 13 / glibc 2.41 (WSL2) |
| linux-x64, `native` (gcc 14 on Debian 13) | glibc ≥ 2.38 | Debian 13 / glibc 2.41 (WSL2) |
| darwin-arm64 / darwin-x64 | macOS ≥ 13.0 (Mach-O `minos`) | run the verify kit on a Mac: `bash run_on_target.sh` |

"Loads on" is not the same as "byte-identical on": the math library (libm) belongs to the target system, so byte-identical
output is only guaranteed where the golden suite has actually been run.
