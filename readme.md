<!--
  NOTE TO SELF (delete before publishing if you like):
  - Replace every "camcimahir" / "ai-boardgames" below with your real GitHub user + repo name.
  - The "Play in browser" link works once GitHub Pages is enabled (Settings -> Pages -> Source: GitHub Actions).
  - Drop your screenshots / GIFs into the media/ folder using the file names referenced below.
-->

# AI Board Games

A collection of board games written from scratch in **C++** with **Dear ImGui**, each driven by a
custom game AI (Negamax with alpha-beta pruning, built on bitboard board representations for speed).

The centerpiece is a **complete chess engine** — full legal move generation, magic bitboards, and a
searching AI — alongside Connect Four, Tic-Tac-Toe, and a programmable space-combat sandbox (AstroBots).
The same codebase runs natively on Windows/macOS/Linux **and compiles to WebAssembly**, so the whole
thing is playable in the browser with no install.

**[Play it in your browser](https://camcimahir.github.io/ai-boardgames/)**
&nbsp;|&nbsp;
**[Download for Windows](https://github.com/camcimahir/ai-boardgames/releases/latest)**
&nbsp;|&nbsp;
**[Build from source](#build-from-source)**

<!-- TODO: add a hero GIF of the chess engine in action (media/chess.gif) -->
<p align="center">
  <img src="media/chess.gif" alt="Chess engine gameplay" width="720">
</p>

## What this project demonstrates

- A **chess engine built from first principles**: bitboards, magic bitboards for sliding pieces,
  full rules (castling, en passant, check/checkmate/stalemate), and Perft-verified move generation.
- **Game-tree search**: Negamax + alpha-beta pruning, with move ordering and evaluation tuning, applied
  across four different games.
- **Performance-minded C++**: 64-bit bitboards and bitwise operations instead of array scans, and an AI
  decoupled from the UI so search isn't bottlenecked by rendering.
- **Cross-platform engineering**: one C++20 / Dear ImGui codebase targeting DirectX 11, OpenGL, and
  WebAssembly/WebGL.
- **Automated delivery**: CI that builds the browser version and deploys it to GitHub Pages, and packages
  a downloadable Windows build.

## How to play

| Option | How | Best for |
| --- | --- | --- |
| Browser (WebAssembly) | Open the **[live demo](https://camcimahir.github.io/ai-boardgames/)** | Trying everything in a few seconds, no install |
| Windows download | Grab the latest **[release `.zip`](https://github.com/camcimahir/ai-boardgames/releases/latest)**, unzip, run `AI-BoardGames.exe` | The fastest native experience (DirectX 11) |
| From source | See [Build from source](#build-from-source) | Reading or modifying the code |

The AI runs entirely on your machine (or in your browser tab). Once the app is open, use the **Settings**
panel to pick a game and a mode (Human vs Human, Human vs AI, and so on).

## Chess (the main project)

<!-- TODO: media/chess.png -->

A complete, rules-accurate chess game with a from-scratch move generator and a searching AI that plays
Black. It is engineered around speed, because a chess AI is limited by how many positions it can evaluate.

**Implemented**
- Full legal move generation: castling, en passant, and check / checkmate / stalemate detection.
- Modes: Human vs Human, and Human (White) vs AI (Black).
- FEN (Forsyth-Edwards Notation) parsing for loading board states and tracking castling rights and
  en-passant squares cheaply.
- Pawns auto-promote to a queen — a deliberate trade-off that keeps the search tree smaller and the AI faster.

**Negamax with alpha-beta pruning**
- Searches the move tree with Negamax (a compact single-function form of minimax) plus alpha-beta pruning
  to discard branches that cannot beat the best line found so far.
- Default search depth is 4 plies (responsive up to ~5). At the depth limit it scores the position with a
  dedicated evaluation function (material and position). A small hard-coded opening book covers the first
  few moves.
- The AI is decoupled from the UI: instead of searching through ImGui widgets, it runs over a plain
  `GameState` structure and bitboards.

**Bitboard architecture**
- The board occupancy and each piece type are stored as 64-bit integer bitboards, and moves are computed
  with bitwise shifts against pre-computed attack tables — far faster than iterating 2D arrays to find
  attacks or detect checks.

**Magic bitboards for sliding pieces**
- Rooks, bishops, and queens can be blocked by other pieces, so simple shifts don't work for them. The
  engine uses magic bitboards — a perfect-hash lookup table (table courtesy of Prof. Graeme Devine) that
  returns the exact attack rays for a sliding piece given the current occupancy in O(1).

**Move-generation verification (Perft / Shannon's algorithm)**
- To prove the move generator is correct, it runs a Perft (performance test) to depth 3, counting all
  reachable positions and catching tricky edge cases (en passant, castling legality, pins, and so on).

## Connect Four

<!-- TODO: media/connect-four.png -->

Classic Connect Four with a genuinely strong AI.

- Modes: Human vs Human, Human vs AI (you first), AI vs Human (AI first).
- Bitboard win detection: the board is encoded in 64-bit integers and all winning lines are checked with
  bit shifts in O(1), instead of scanning arrays.
- Negamax + alpha-beta, searching up to 12 plies at a playable speed. The string-based game logic is
  converted to bitboards at the root of the search for fast win checks.
- Move ordering from the center outward (`3, 2, 4, 1, 5, 0, 6`) so pruning takes effect earlier.
- A positional score table breaks ties toward stronger central squares when the search bottoms out before
  a forced result, plus a small hard-coded opening for the AI's first two moves.

## Tic-Tac-Toe

<!-- TODO: media/tictactoe.png -->

A minimal, unbeatable Tic-Tac-Toe — a clean demonstration of the Negamax idea that powers the larger games.

- The AI plays second and uses Negamax to search every reachable end state, scoring +1 / -1 / 0 for
  win / loss / draw, so it never loses.
- Built on the engine's `BitHolder` (logic) and `Bit` (visuals) grid, with an 8-line lookup table for win
  detection.

## AstroBots

<!-- TODO: media/astrobots.gif -->

A real-time space-combat sandbox where each ship is "programmed" with a tiny domain-specific language
(DSL). You write a `SetupShip()` function that emits a sequence of opcodes (scan, thrust, turn, fire), and
the arena runs your bytecode every turn. Last ship alive wins.

- A toroidal (edge-wrapping) 2048x2048 arena with ships, drifting asteroids, fuel, and cooldowns.
- A small bytecode VM / interpreter with conditionals and flow control, under a 30-point "script cost" budget.

Full opcode reference, DSL macros, and bot-writing tips are in **[docs/ASTROBOTS.md](docs/ASTROBOTS.md)**.

## Tech overview

- Language / UI: C++20, Dear ImGui (docking branch).
- Rendering: DirectX 11 on Windows; OpenGL 3 / GLFW on macOS and Linux; WebGL via Emscripten in the
  browser — all from one codebase.
- AI: Negamax + alpha-beta pruning across all games, with bitboard board representations for speed.
- Chess-specific: magic bitboards, FEN parsing, Perft verification.
- Build: CMake, with CI that produces both the web build and a Windows download automatically.

## Build from source

### Windows (native, DirectX 11)
Requires Visual Studio 2022 (Desktop C++ workload) and CMake.

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
# Run it (resources are copied next to the exe automatically):
./build/Release/demo.exe
```

Or open the folder in Visual Studio 2022 — it picks up `CMakePresets.json` and the
`Visual Studio 2022 - x64` preset directly.

### macOS / Linux (OpenGL + GLFW)
Requires CMake, a C++20 compiler, and GLFW + OpenGL development packages.

```bash
cmake -B build
cmake --build build -j
./build/demo
```

### Web (WebAssembly)
Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html).

```bash
emcmake cmake -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j
# Serve the folder (file:// won't load .wasm), then open index.html:
python -m http.server --directory build-web 8080
```

You normally don't need to do this by hand — pushing to `main` builds and deploys the web version to
GitHub Pages automatically (see `.github/workflows/deploy-web.yml`).

## Releases and deployment (CI)

This repo ships two GitHub Actions workflows:

- `deploy-web.yml` — builds the Emscripten / WebAssembly bundle and publishes it to GitHub Pages on every
  push to `main`. (Enable once under Settings -> Pages -> Source: GitHub Actions.)
- `windows-release.yml` — builds the native Windows app, packages `AI-BoardGames.exe` and `resources/`
  into a `.zip`, uploads it as a build artifact, and attaches it to a GitHub Release when you push a
  version tag (for example `git tag v1.0.0 && git push origin v1.0.0`).

## Credits

- Boardgame engine framework, magic-bitboard tables, and course structure: Professor Graeme Devine
  (CMPM 123, UC Santa Cruz).
- UI: [Dear ImGui](https://github.com/ocornut/imgui) by Omar Cornut.
- Game design, AI, chess engine, and the cross-platform / WebAssembly work: me.
