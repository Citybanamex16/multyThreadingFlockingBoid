# 🐦 Multi-Threaded Flocking Simulation

> An interactive 2D simulation of collective agent behavior using concurrent systems in C++17 and SFML.

---

**Author:** Carlos Delgado Contreras  
**Student ID:** A01712819  
**Date:** May 2026  
**Course:** Computational Methods / Advanced Programming

---

## Table of Contents

- [Introduction](#introduction)
- [Project Status](#project-status)
- [Installation & Execution](#installation--execution)
  - [macOS (Local / Portable)](#macos-local--portable)
  - [Windows (Standard Setup)](#windows-standard-setup)

---

## Introduction

This project is an interactive 2D simulation of the **Flocking algorithm** originally developed by Craig Reynolds (Boids). It models the collective behavior of autonomous agents based on three fundamental physical rules:

| Rule | Description |
|------|-------------|
| **Cohesion** | Stay close to the group |
| **Alignment** | Move in the same direction as your neighbors |
| **Separation** | Avoid colliding with other agents |

### Architecture

The core of the project is built on a **Concurrent Systems architecture in C++17**, featuring:

- A custom **ThreadPool** with persistent execution threads based on condition variables and mutual exclusion (`std::mutex`)
- A **Fork-Join synchronization pattern** combined with **Double-Buffer attribute updates**
- Parallel delegation of massive physical force calculations for all agents
- A dedicated main thread for the graphic rendering cycle using the **SFML** library

---

## Project Status

### ✅ What's Working

- **Multithreaded Architecture** — Functional ThreadPool that efficiently manages the concurrent task queue.
- **Stable Physical Model** — Implementation of Reynolds' three forces, optimized with weight multipliers, Velocity Clamping (to prevent exponential explosions), and a toroidal space (mirror effect on the edges of an 800×600 screen).
- **Real-Time Rendering** — SFML-based Game Loop integration, where worker threads compute the current frame in parallel while the main thread draws agents fluidly.

### 🔜 Pending Work

- **Cross-Platform Portability Testing** — Due to hardware limitations on the development machine (an older Mac model), compilation was done by linking specific legacy binaries locally. Validation on other computers (modern hardware architectures and other operating systems) is pending to ensure standard dependencies link correctly without custom configurations.
- **Data Structure Optimization** — Neighbor search currently runs at $O(N^2)$ complexity. A spatial partitioning strategy (Spatial Hashing or Quadtree) is planned to improve performance with thousands of agents.
- **User Interface (UI)** — On-screen controls to adjust force weights in real time.

---

## Installation & Execution

The project is designed to be portable, bundling the necessary headers and libraries locally for legacy systems.

### macOS (Local / Portable)

> ⚠️ Due to macOS's strict security policies (Gatekeeper), you must clear the system quarantine flag from the included local binaries before compiling.

**1. Project Structure**

Ensure the `include`, `lib`, and `Frameworks` folders are at the root alongside the source code.

**2. Remove Apple Quarantine**

```bash
xattr -d com.apple.quarantine ./lib/*.dylib
xattr -d -r com.apple.quarantine ./Frameworks/freetype.framework
```

**3. Compile**

```bash
g++ -std=c++17 GraficMultiThreadFlocking.cpp \
    -I./include -L./lib \
    -lsfml-graphics -lsfml-window -lsfml-system \
    -pthread -o boids_sim
```

**4. Run**

```bash
DYLD_FRAMEWORK_PATH=$PWD/Frameworks DYLD_LIBRARY_PATH=$PWD/lib ./boids_sim
```

---

### Windows (Standard Setup)

**1.** Download and install SFML compatible with your compiler (MinGW or MSVC).

**2.** Configure the include (`-I`) and library (`-L`) paths in your IDE or build script.

**3.** Link the corresponding dynamic libraries:

```
-lsfml-graphics -lsfml-window -lsfml-system
```

**4.** Copy the corresponding SFML `.dll` files into the same folder as the generated `.exe`.
