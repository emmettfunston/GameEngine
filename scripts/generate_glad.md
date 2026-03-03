## Generating glad for macOS OpenGL 4.1 core

This project vendors a **pre-generated glad loader** in `third_party/glad/`.

### Option A (recommended): glad2 Python package

```bash
python3 -m pip install --upgrade pip
python3 -m pip install glad2
python3 scripts/generate_glad.py
```

### Option B: glad web generator

Use the glad generator (glad2) and download for:

- **Specification**: OpenGL (gl)
- **API**: `gl:core=4.1`
- **Language**: C
- **Loader**: enabled
- **Extensions**: `GL_KHR_debug` (optional, helps debug)

Place outputs:

- `include/glad/gl.h` -> `third_party/glad/include/glad/gl.h`
- `include/KHR/khrplatform.h` -> `third_party/glad/include/KHR/khrplatform.h`
- `src/gl.c` -> `third_party/glad/src/gl.c`


