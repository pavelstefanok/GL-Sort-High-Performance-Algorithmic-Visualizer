# GL-Sort: High-Performance Algorithmic Visualizer


A real-time 3D sorting visualizer built with **Modern OpenGL (4.0+)**, designed with a modular architecture and hardware-accelerated rendering.

---
https://github.com/user-attachments/assets/194c467f-b15f-446b-9dd7-2c71001dc3c6

---
## Download & Run
You can download the compiled version (no setup required) from the **[Latest Release](https://github.com/pavelstefanok/GL-Sort-High-Performance-Algorithmic-Visualizer/releases)**.

1. Download the `GL-Sort-vx.x.zip` file.
2. Extract the archive (ensure `algorithm.dll` and all system `.dll` files are in the same folder as `app.exe`).
3. Run `app.exe`.
   * **Controls:** * `SPACE`: Reshuffle the array with animation.
     * `S`: Trigger **Bubble Sort** from the external DLL.
     * `Orbit Camera`: Automatic rotation enabled by default.

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
* **API:** Win32 API (Dynamic Library Loading, Message Polling)
* **Compiler:** MinGW-w64 (MSYS2)

---

## Project Structure
* `src/`: Core engine logic (`main.cpp`)
* `include/`: Shared header files and sorting interface.
* `algorithm.dll`: The modular library containing the sorting logic.
---

## Work in Progress
This project is an evolving platform. Upcoming features include:

* [ ] **Advanced Algorithms:** Adding QuickSort, MergeSort, RadixSort, etc.. to the DLL.
* [ ] **Sonification:** Thinking of integrating **MATLAB** to generate real-time audio frequencies based on array values.
* [ ] **Interactive UI:** On-screen controls for simulation speed and dataset size.

---

## Author
**Pavel Ștefan**
