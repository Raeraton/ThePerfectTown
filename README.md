# The Perfect Town

A C++ city and population simulation exploring NPC relationships, weekly schedules, workplaces, residences, and background traffic routing.

<p align="center">
  <img src="https://img.shields.io/badge/C%2B%2B-17-blue.svg" alt="C++17">
  <img src="https://img.shields.io/badge/build-clang%2B%2B-orange.svg" alt="Clang++">
  <img src="https://img.shields.io/badge/platform-Linux-lightgrey.svg" alt="Linux">
</p>

## ✨ Features

- **NPC world simulation** — creates a population and advances residents through a 168-hour week
- **Relationships** — models friends, partners, family, coworkers, roommates, and more
- **Residences and workplaces** — assigns inhabitants to apartments and workers to jobs
- **Weekly schedules** — tracks when people are expected to work, travel, or participate in events
- **Background traffic** — builds weighted road graphs and calculates shortest paths
- **Trip timing** — records travel intervals across graph edges
- **Performance profiling** — includes a ready-made Linux `perf` and FlameGraph script
- **Fixed-point and container utilities** — lightweight numerical and rotating-buffer helpers

## 🧱 Project structure

```text
.
├── game/
│   ├── include/     Public headers and simulation types
│   └── src/         Implementation and benchmark entry point
├── tests/           Graph and rotating-buffer tests
├── app              Compiled executable
├── compile.sh       Build script
├── flame-graph.sh   Linux profiling helper
└── first_names.txt
└── last_names.txt
```

## 🚀 Build

A working Clang++ installation is required.

```bash
./compile.sh
```

The build script compiles every `game/src/*.cpp` file with `-O3`, debug symbols, and the project headers, producing an executable named `app`.

## 🎮 Run

The executable has two benchmark modes:

### NPC simulation

```bash
./app npc [population] [iterations]
```

- `population` defaults to `1000`
- `iterations` defaults to `120`

Example:

```bash
./app npc 10000 168
```

The simulation prints initialization and execution timings and prints the world only for populations up to 1,000,000.

### Road graph benchmark

```bash
./app graph [width] [start] [end] [range]
```

- `width` — width and height of the generated grid; default: `10`
- `start` — starting node index; default: `99`
- `end` — destination node index; default: `0`
- `range` — random coordinate range modifier; default: `1`

Example:

```bash
./app graph 20 399 0 10
```

The command prints a text route, trip timing details, and graph generation timings.

## 🧪 Tests

The test sources are standalone C++ programs:

```bash
clang++ -std=c++17 -Igame/include tests/rotating_buffer_test.cpp -o rotating_buffer_test
./rotating_buffer_test

clang++ -std=c++17 -Igame/include \
  tests/bg_graph_test.cpp \
  game/src/background_traffic.cpp \
  game/src/n_npc_behavior.cpp \
  game/src/fixedpoint_num.cpp \
  -o bg_graph_test
./bg_graph_test
```

Assertions are used for validation, so build and run each test with assertions enabled.

## 🔥 Profiling

On Linux, `flame-graph.sh` records `app` with `perf` and converts the result into a FlameGraph SVG:

```bash
# Start the application in another terminal
./app npc 10000 168

# Record and generate perf.svg
./flame-graph.sh
```

A local FlameGraph checkout is expected at `~/scripts/FlameGraph`.

## 📄 License

No license file is currently included in this repository. Add a license before redistributing or deploying the project.
