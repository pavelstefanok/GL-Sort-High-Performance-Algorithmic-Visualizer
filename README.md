# GL-Sort: High-Performance Algorithmic Visualizer


A real-time 3D sorting visualizer built with **Modern OpenGL (4.0+)**, designed with a modular architecture and hardware-accelerated rendering.

---
https://github.com/user-attachments/assets/194c467f-b15f-446b-9dd7-2c71001dc3c6

---
## Download & Run
You can download the compiled version (no setup required) from the **[Latest Release](https://github.com/pavelstefanok/GL-Sort-High-Performance-Algorithmic-Visualizer/releases)**.

1. Download the `GL-Sort.rar` file.
2. Extract the archive.
3. Run `app.exe`.
   * **Controls:** * `SPACE`: Reshuffle the array with animation.
     * `Orbit Camera`: Automatic rotation enabled by default.
     * The Yellow bar is the .

---
##  Visual Feedback & Legend
The visualizer uses a dynamic color-coding system to represent the algorithm's internal state in real-time:

* 🟡 **Yellow:** Represents the **Active Element** or **Pivot** currently being processed.
* 🟣 **Magenta:** Indicates the **Comparison Target** (the element being compared against the active one).
* 🟢 **Green:** Highlighted during a **Swap** operation or the final **Success Animation**.
* 🔹 **Cyan Gradient:** The default state, where color intensity is mapped to the element's height.
---
## Key Technical Features
* **GPU Instancing:** Uses `glDrawArraysInstanced` to render the entire array in a single draw call, significantly reducing CPU overhead.
* **Embedded-Ready Core:**
    * **Iterative & Stack-Safe:** Zero recursion to prevent **Stack Overflow**, ensuring reliability in memory-constrained environments.
    * **Deterministic Types:** Uses `int32_t` / `uint32_t` (`stdint.h`) for cross-platform binary compatibility.
    * **Zero Heap Allocation:** No `malloc` or `new` calls during execution to eliminate memory fragmentation.
* **Dynamic DLL Loading:** Algorithms are decoupled from the main engine. The app uses Win32 `LoadLibrary` to call sorting functions from `algorithm.dll` at runtime.
* **Optimized Buffer Updates:** Instead of re-uploading the whole array, the engine uses `glBufferSubData` to update only the heights of the two bars being swapped.
* **Shader-Based Effects:** Shaders handle color interpolation (based on height) and the "Green Wave" verification.



---

## Tech Stack
* **Language:** C++ / C (Embedded-style logic)
* **Graphics:** OpenGL 4.0 (GLEW, FreeGLUT)
* **Shaders:** GLSL
* **API:** Win32 API (Dynamic Library Loading, Message Polling)
* **Compiler:** MinGW-w64 (MSYS2)

---

##  Project Structure
* `src/`: Core engine logic, UI mapping, and input handling.
* `shaders/`: **Critical** custom GLSL files for vertex transformations and height-based color interpolation. 
* `algorithm.dll`: Modular library containing iterative (non-recursive) sorting logic.
* `include/`: Shared headers and the sorting interface for the DLL.

---

##  Roadmap

* [x] **UI:** Interactive buttons and dual-slider system for Size and Speed control.

* [x] **Coordinate Mapping:** Resolution-independent HUD and UI elements.

* [ ] **More Algorithms**.

* [ ] **Sonification:** Audio frequency generation based on array values for real-time auditory feedback.

* [ ] **etc**.



---

## Author
**Pavel Ștefan**
