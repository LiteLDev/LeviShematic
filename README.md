# LeviShematic

**Bedrock Edition Projection Mod** — A Litematica-style building projection tool based on [LeviLamina](https://github.com/LiteLDev/LeviLamina)

Ports the core functionalities of the Java Edition [Litematica](https://github.com/maruohon/litematica) mod to Minecraft Bedrock Edition. It supports loading building blueprints in the `.litematic` format and renders projections as an overlay of translucent ghost blocks in the game world, helping players to accurately reconstruct buildings.

-----

## ✨ Features

### Implemented ✅

  - **Full `.litematic` file parsing** — Custom Big-Endian NBT parser (supports `TAG_Long_Array`), GZip decompression + recursive NBT parsing.
  - **Java → BE Block Mapping** — Built-in difference mapping table for 150+ Java blockstates to Bedrock `Block*`.
  - **Projection Ghost Block Rendering** — By hooking `RenderChunkBuilder::_sortBlocks` and `BlockTessellator::tessellateInWorld`, projection blocks are rendered as a translucent color overlay in the game world.
  - **Projection Placement Management** — Supports loading multiple projections simultaneously with independent toggle/hide controls.
  - **Rotation and Mirroring** — Supports CW90°/CCW90°/180° rotation and X/Z axis mirror transformations.
  - **Sub-region Support** — Each sub-region can be independently configured with offset, rotation, and mirroring.
  - **Comprehensive Command System** — 13+ commands/subcommands covering load, move, rotate, mirror, list, remove operations, etc.
  - **Lock-free Render Architecture** — Based on the `atomic shared_ptr` snapshot mechanism of `ProjectionState`, allowing lock-free reads by the render thread.

### In Development 🚧

  - **Selection System** — `AreaSelection` + `Box` to select regions in the world for saving buildings as Schematics.
  - **Schematic Verifier** — `SchematicVerifier` compares the projection with the real world chunk by chunk, highlighting incorrect/missing/extra blocks.
  - **Material List Statistics** — Calculates the number of blocks required for the projection, supporting comparison with the player's inventory.
  - **GUI Interface** — Graphical user interface based on the LL GUI API.
  - **Hotkey Binding** — Keyboard shortcuts to control the projection (move, rotate, toggle render, etc.).

-----

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      LeviShematic                               │
│                                                                 │
│  ┌──────────────┐    ┌───────────────┐    ┌────────────────┐    │
│  │Command System│    │ Input System  │    │   GUI System   │    │
│  │  /schem ...  │    │Hotkey Handling│    │   (GUI API)    │    │
│  └──────┬───────┘    └──────┬────────┘    └────────────────┘    │
│         │                   │                                   │
│  ┌──────▼───────────────────▼──────────────────────────────┐    │
│  │                 DataManager (Core Singleton)            │    │
│  │  SelectionManager | PlacementManager | SchematicHolder  │    │
│  └──┬────────────┬──────────────┬─────────────┬────────────┘    │
│     │            │              │             │                 │
│  ┌──▼───┐  ┌─────▼───┐  ┌──────▼───┐  ┌──────▼───┐              │
│  │Select│  │Schematic│  │  Render  │  │ Verifier │              │
│  │Layer │  │ Format  │  │  Layer   │  │  Layer   │              │
│  └──────┘  └─────────┘  └──────────┘  └──────────┘              │
│                                                                 │
│  ┌─────────────────────────────────────────────────────────┐    │
│  │           Low-level Hook Layer (Render Injection)       │    │
│  │  RenderChunkBuilder Hook | BlockTessellator Hook        │    │
│  └─────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────┘
```

### Core Data Flow

```
.litematic File → GZip Decompression → NBT Parsing → LitematicSchematic
                                              ↓
                                    SchematicPlacement (Coordinates + Rotation + Mirroring)
                                              ↓
                                    PlacementManager::rebuildProjection()
                                              ↓
                                    ProjectionState (atomic snapshot update)
                                              ↓
                                ┌─────────────┴───────────────┐
                                ↓                             ↓
                    RenderChunkBuilder Hook          BlockTessellator Hook
                   (Inject blocks into BLEND queue)  (Translucent color overlay)
                                              ↓
                          Projection ghost block rendered to screen
```

-----

## 📁 Project Structure

```
src/levishematic/
├── LeviShematic.h/cpp            ← Mod entry point (LL registration)
├── core/
│   └── DataManager.h/cpp         ← Global state hub singleton
├── schematic/
│   ├── LitematicSchematic.h/cpp  ← Full .litematic parsing (Custom NBT parser)
│   ├── SchematicMetadata.h       ← Metadata (Name/Author/Timestamp)
│   ├── BlockStateContainer.h/cpp ← Packed bit compressed block storage unpacking
│   ├── BlockStatePalette.h/cpp   ← Java blockstate to BE Block* mapping
│   ├── NBTReader.h/cpp           ← GZip + NBT file read/write
│   ├── SchematicTransform.h/cpp  ← Rotation/Mirror coordinate transformation
│   └── placement/
│       ├── SchematicPlacement.h/cpp  ← Projection placement instance
│       └── PlacementManager.h/cpp    ← Multi-projection manager
├── render/
│   ├── ProjectionRenderer.h/cpp  ← Projection state & snapshot (lock-free)
│   ├── WorldSchematic.h/cpp      ← Virtual world block cache
│   └── ChunkRenderCache.h/cpp    ← SubChunk render cache
├── hook/
│   ├── RenderHook.h/cpp          ← Render pipeline Hook (Block injection + Color control)
│   └── TickHook.h/cpp            ← Client Tick Hook (Init + Scheduling)
├── command/
│   └── Command.h/cpp             ← Comprehensive command system (13+ subcommands)
├── input/
│   └── InputHandler.h/cpp        ← Input/Hotkey handling (Interface reserved)
├── selection/
│   ├── AreaSelection.h/cpp       ← Area selection
│   ├── Box.h/cpp                 ← Selection box
│   └── SelectionManager.h/cpp    ← Selection manager
├── verifier/
│   └── SchematicVerifier.h/cpp   ← Schematic verifier
├── util/
│   ├── BlockUtils.h/cpp          ← Java to BE block name mapping (150+ entries)
│   └── PositionUtils.h           ← SubChunk coordinate encoding utility
├── materials/                    ← Material list (To be implemented)
├── scheduler/                    ← Async task scheduling (To be implemented)
└── gui/screens/                  ← GUI screens (To be implemented)
```

-----

## 📋 Command List

All commands are prefixed with `/schem`:

| Command | Description | Example |
|------|------|------|
| `/schem load <file> [x y z]` | Load Schematic and place at specified/player position | `/schem load house.litematic 0 64 0` |
| `/schem move <dx> <dy> <dz>` | Move the currently selected projection | `/schem move 10 0 5` |
| `/schem origin <x> <y> <z>` | Set the projection origin coordinates | `/schem origin 100 64 200` |
| `/schem rotate <direction>` | Rotate the current projection (cw90/ccw90/r180) | `/schem rotate cw90` |
| `/schem mirror <axis>` | Mirror the current projection (x/z/none) | `/schem mirror x` |
| `/schem list` | List all loaded projections | `/schem list` |
| `/schem select <id>` | Select the projection with the specified ID | `/schem select 2` |
| `/schem remove [id]` | Remove the specified/currently selected projection | `/schem remove` |
| `/schem toggle [render]` | Toggle projection enable/render visibility | `/schem toggle render` |
| `/schem info` | Show detailed information of the current projection | `/schem info` |
| `/schem files` | List available .litematic files | `/schem files` |
| `/schem reset` | Reset the current projection transformation | `/schem reset` |
| `/schem clear` | Clear all projections | `/schem clear` |

-----

## 🔧 Build Guide

### Requirements

  - **OS**: Windows (x64)
  - **Compiler**: MSVC (Visual Studio 2022+)
  - **Build Tool**: [xmake](https://xmake.io/)
  - **Dependencies**:
      - [LeviLamina](https://github.com/LiteLDev/LeviLamina) — BDS Mod Loader
      - [zlib 1.3.2](https://zlib.net/) — GZip Decompression

### Build Steps

```bash
# 1. Clone the repository
git clone <repo-url>
cd LeviShematic

# 2. Configure the project (Client Mod)
xmake f -y -p windows -a x64 -m release --target_type=client

# 3. Build
xmake

# Build artifacts are located in the bin/ directory
```

> **Note**: This project is a **Client Mod** and requires LeviLamina with client mod loading support.

### Installation

Copy the build artifacts from the `bin/` directory to LeviLamina's `mods/` directory. Place `.litematic` files in the `schematics/` directory to load them using the `/schem load` command.

-----

## 🗺️ Roadmap

| Phase | Content | Status |
|------|------|------|
| Phase 1 | Core Render Hook (ProjectionState / RenderChunkBuilder / BlockTessellator) | ✅ Completed |
| Phase 2 | Schematic Loading (NBT Parsing / BlockStatePalette / Block Mapping) | ✅ Completed |
| Phase 3 | Placement Management (SchematicPlacement / PlacementManager / Command System) | ✅ Completed |
| Phase 4 | Selection and Saving (AreaSelection / Mouse Selection Hook / Schematic Writing) | 🚧 In Development |
| Phase 5 | Verifier (WorldSchematic Cache / Chunk by Chunk Comparison / Error Highlighting) | 📋 Planned |
| Phase 6 | Material List and GUI (MaterialList / LL GUI Display / Config Persistence) | 📋 Planned |

-----

## 🔑 Technical Highlights

  - **Custom Big-Endian NBT Parser** — LL native API does not support `TAG_Long_Array` (type 12). The project implements a complete custom parser to correctly read the Litematica format.
  - **Lock-free Render Thread Safety** — Uses `std::atomic<std::shared_ptr>` to implement lock-free reading of ProjectionState snapshots, resulting in zero wait time for the render thread.
  - **Packed Bit Compression Unpacking** — Implements Litematica's packed bit compressed block storage unpacking algorithm, supporting bit extraction across `long` boundaries.
  - **Java → BE Block State Conversion** — Parses Java Edition blockstate strings (e.g., `minecraft:oak_log[axis=y]`) and maps them to BE `Block*` objects.

-----

## 📄 License

[CC0-1.0](https://www.google.com/search?q=LICENSE) — Public Domain, free to use.

-----

## 🤝 Contributing

Issues and Pull Requests are welcome\!

If you have any questions or suggestions, please discuss them by creating an Issue.

## 🤖 Note:

All content in this repository is generated by AI.