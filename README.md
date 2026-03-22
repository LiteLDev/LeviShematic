# LeviShematic

Bedrock Edition projection mod based on [LeviLamina](https://github.com/LiteLDev/LeviLamina).

LeviShematic uses `.mcstructure` as its only blueprint format. It can load saved structures, render translucent projection blocks in the world, and save in-world selections back to `.mcstructure`.

## Features

- `.mcstructure` load and save based on Bedrock `StructureTemplate`
- Ghost block projection rendering
- Placement move, rotate, mirror, hide, enable/disable
- Multi-placement management
- Selection corners and wireframe rendering
- Command-driven workflow via `/schem`

## Commands

All commands are prefixed with `/schem`.

| Command | Description |
|------|------|
| `/schem load <file> [x y z]` | Load a `.mcstructure` projection |
| `/schem move <dx> <dy> <dz>` | Move current placement |
| `/schem origin <x y z>` | Set placement origin |
| `/schem rotate <cw90\|ccw90\|r180>` | Rotate current placement |
| `/schem mirror <x\|z\|none>` | Mirror current placement |
| `/schem list` | List all placements |
| `/schem select <id>` | Select a placement |
| `/schem remove [id]` | Remove placement |
| `/schem toggle [render]` | Toggle placement or render visibility |
| `/schem info` | Show current placement info |
| `/schem files` | List available `.mcstructure` files |
| `/schem reset` | Reset transform |
| `/schem clear` | Clear all placements |
| `/schem pos1 [x y z]` | Set selection corner 1 |
| `/schem pos2 [x y z]` | Set selection corner 2 |
| `/schem save <name>` | Save selection as `.mcstructure` |
| `/schem selection clear\|mode\|info` | Manage selection |

## Project Structure

```
src/levishematic/
├── core/                         # global state hub
├── command/                      # command registration and handlers
├── hook/                         # render/tick/selection hooks
├── input/                        # hotkey/input layer (reserved)
├── render/                       # projection state and render helpers
├── schematic/placement/          # placement loading and transform logic
├── selection/                    # selection and .mcstructure saving
├── util/                         # position/subchunk utilities
└── verifier/                     # verifier placeholder
```

## Build

Requirements:

- Windows x64
- Visual Studio 2022+
- [xmake](https://xmake.io/)
- [LeviLamina](https://github.com/LiteLDev/LeviLamina)

Build steps:

```bash
xmake f -y -p windows -a x64 -m release --target_type=client
xmake
```

Install the built artifacts into LeviLamina's `mods/` directory. Put `.mcstructure` files into the `schematics/` directory, then load them with `/schem load`.

## Status

- Phase 1: Core render hook — complete
- Phase 2: `.mcstructure` loading and placement — complete
- Phase 3: Placement management and commands — complete
- Phase 4: Selection and save — complete
- Phase 5: Verifier — planned
- Phase 6: Material list and GUI — planned

## Notes

Current build may still fail at final link stage in some environments because of external Bedrock/Hook symbol availability. The project source itself is now aligned around `.mcstructure` only.
