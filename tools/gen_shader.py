#!/usr/bin/env python3
"""
Offline single-shader compiler. Compiles one GLSL shader to SPIR-V at a time.

Handles #version injection (adds 450 if missing) and #include resolution
via glslc's built-in preprocessor.

Usage:
    python tools/gen_shader.py <shader_file> -o <output.spv> [-D DEFINE ...] [-I dir ...]
"""

import os
import sys
import subprocess
import argparse
from pathlib import Path


# -- glslc discovery ---------------------------------------------------------

def find_glslc() -> str:
    """Locate glslc via VULKAN_SDK env var or PATH."""
    import shutil

    sdk = os.environ.get("VULKAN_SDK")
    if sdk:
        sdk_path = Path(sdk)
        if sys.platform == "win32":
            candidate = sdk_path / "Bin" / "glslc.exe"
        else:
            candidate = sdk_path / "bin" / "glslc"
        if candidate.is_file():
            return str(candidate.resolve())

    found = shutil.which("glslc")
    if found:
        return found

    raise FileNotFoundError(
        "glslc not found. Set VULKAN_SDK or ensure glslc is in PATH."
    )


# -- Shader stage detection --------------------------------------------------

STAGE_MAP = {
    ".vert": "vertex",
    ".frag": "fragment",
    ".comp": "compute",
    ".geom": "geometry",
    ".tesc": "tesscontrol",
    ".tese": "tesseval",
}


def shader_stage_flag(path: Path) -> str:
    stage = STAGE_MAP.get(path.suffix.lower(), "vertex")
    return f"-fshader-stage={stage}"


# -- Source preprocessing ----------------------------------------------------

def read_shader_source(path: Path) -> str:
    """Read source, prepending #version 450 if no #version directive exists."""
    source = path.read_text(encoding="utf-8")
    has_version = any(line.strip().startswith("#version") for line in source.splitlines())
    if not has_version:
        source = "#version 450\n" + source
    return source


# -- Compilation -------------------------------------------------------------

def compile_shader(shader_path: Path, output_path: Path, defines: list[str],
                   include_dirs: list[str], glslc_path: str,
                   force: bool = False) -> bool:
    """Compile one GLSL shader to SPIR-V via glslc stdin pipe.

    Returns True on success.
    """
    if output_path.exists() and not force:
        return True

    output_path.parent.mkdir(parents=True, exist_ok=True)
    source = read_shader_source(shader_path)

    cmd = [glslc_path, shader_stage_flag(shader_path)]
    cmd.extend(defines)
    for d in include_dirs:
        cmd.extend(["-I", d])
    cmd.extend(["-o", str(output_path), "-"])

    try:
        result = subprocess.run(cmd, input=source, capture_output=True,
                                text=True, timeout=120)
        if result.returncode != 0:
            print(f"FAIL: {shader_path.name} -> {output_path.name}")
            err = result.stderr.strip()
            if err:
                print(f"  {err}")
            return False
        print(f"OK: {output_path.name}")
        return True
    except subprocess.TimeoutExpired:
        print(f"TIMEOUT: {shader_path.name}")
        return False
    except FileNotFoundError:
        print(f"glslc not found: {glslc_path}")
        sys.exit(1)


# -- CLI ---------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Compile one GLSL shader to SPIR-V."
    )
    parser.add_argument("shader", type=Path,
                        help="Path to .vert/.frag/.comp shader source")
    parser.add_argument("-o", "--output", type=Path, required=True,
                        help="Output .spv path")
    parser.add_argument("-D", "--define", action="append", default=[],
                        dest="defines",
                        help="Preprocessor define (-D NAME or -D NAME=VAL)")
    parser.add_argument("-I", action="append", default=[],
                        dest="include_dirs",
                        help="Include directory for #include resolution")
    parser.add_argument("--glslc", default=None,
                        help="glslc path (default: auto-detect)")
    parser.add_argument("--force", action="store_true",
                        help="Recompile even if .spv exists")

    args = parser.parse_args()

    # Resolve glslc
    if args.glslc:
        glslc_path = args.glslc
    else:
        try:
            glslc_path = find_glslc()
        except FileNotFoundError as e:
            print(f"ERROR: {e}")
            sys.exit(1)

    # If no -I given, default to shader's parent dir
    include_dirs = list(args.include_dirs) if args.include_dirs else []
    if not include_dirs:
        include_dirs.append(str(args.shader.parent.resolve()))

    # Format defines as -D flags for glslc
    defines = []
    for d in args.defines:
        d = d.strip()
        if not d.startswith("-D"):
            d = "-D" + d
        defines.append(d)

    ok = compile_shader(args.shader, args.output, defines,
                        include_dirs, glslc_path, args.force)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
