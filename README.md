# Venice External - FiveM

Fully external FiveM cheat. No injection, no hooks, no ASI.

## Build
1. Install **Visual Studio 2022** (Desktop C++ workload) and **CMake** (add to PATH).
2. Run `build.bat`.
3. `FiveMExternal.exe` appears in the project root.

## Run
1. Start FiveM, join a server.
2. Run `FiveMExternal.exe` (before or after the game - it retries).
3. Press **INSERT** to toggle the menu.

## Notes
- Offsets live in `src/core/offsets.h`. When FiveM updates, retune from the
  **MISC -> Debug / Offsets** tab first (no recompile needed) before editing the header.
- If ESP lines up but skeletons look wrong, cycle **Bone Layout Mode**.
- `Visible Only` uses the entity invisible flag; leave it off if everything disappears.
- Configs save to `./configs/`.