from __future__ import annotations

import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUT_DIR = ROOT / "third_party" / "glad"
EXTENSIONS_FILE = Path(__file__).with_name("glad_extensions.txt")


def main() -> int:
    """
    Generates glad loader files into third_party/glad for OpenGL 4.1 core profile.

    Requires: `pip install glad2`
    """
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    tmp = ROOT / "build" / "glad_gen_tmp"
    if tmp.exists():
        shutil.rmtree(tmp)
    tmp.mkdir(parents=True, exist_ok=True)

    cmd = [
        "python3",
        "-m",
        "glad",
        "--api",
        "gl:core=4.1",
        "--extensions",
        str(EXTENSIONS_FILE),
        "--out-path",
        str(tmp),
        "c",
        "--loader",
    ]

    print("Running:", " ".join(cmd))
    subprocess.check_call(cmd)

    # glad2 writes into {out}/include and {out}/src.
    # Current output file names are `gl.c` and `gl.h`.
    src = tmp / "src" / "gl.c"
    inc_glad = tmp / "include" / "glad" / "gl.h"
    inc_khr = tmp / "include" / "KHR" / "khrplatform.h"

    if not src.exists() or not inc_glad.exists() or not inc_khr.exists():
        raise RuntimeError("glad generation output missing; check glad2 version/output layout")

    (OUT_DIR / "src").mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "include" / "glad").mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "include" / "KHR").mkdir(parents=True, exist_ok=True)

    shutil.copy2(src, OUT_DIR / "src" / "gl.c")
    shutil.copy2(inc_glad, OUT_DIR / "include" / "glad" / "gl.h")
    shutil.copy2(inc_khr, OUT_DIR / "include" / "KHR" / "khrplatform.h")

    print("Wrote:")
    print(" -", OUT_DIR / "src" / "gl.c")
    print(" -", OUT_DIR / "include" / "glad" / "gl.h")
    print(" -", OUT_DIR / "include" / "KHR" / "khrplatform.h")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())


