// Helper to wrap a desktop-style "while (!done) { ... }" render loop so the same
// source also works under Emscripten, which requires a callback-driven main loop.
//
// Usage:
//   #ifdef __EMSCRIPTEN__
//   EMSCRIPTEN_MAINLOOP_BEGIN
//   #else
//   while (!glfwWindowShouldClose(window))
//   #endif
//   {
//       ... your per-frame code ...
//   }
//   #ifdef __EMSCRIPTEN__
//   EMSCRIPTEN_MAINLOOP_END;
//   #endif
//
// This mirrors the upstream Dear ImGui helper of the same name.

#pragma once

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <functional>
static std::function<void()> MainLoopForEmscriptenP;
static void MainLoopForEmscripten() { MainLoopForEmscriptenP(); }
#define EMSCRIPTEN_MAINLOOP_BEGIN MainLoopForEmscriptenP = [&]()
#define EMSCRIPTEN_MAINLOOP_END ; emscripten_set_main_loop(MainLoopForEmscripten, 0, true)
#else
#define EMSCRIPTEN_MAINLOOP_BEGIN
#define EMSCRIPTEN_MAINLOOP_END
#endif
