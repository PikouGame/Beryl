# Beryl helper: pk_s.dll

Source code of the one compiled file shipped with **Beryl - LOD Optimisation**, an unofficial fan-made mod for Palworld. Everything else in the mod is plain Lua and a .pak file.

## What it does

Beryl's Lua code cannot write a model's level-of-detail thresholds by itself: UE4SS gives Lua no way to do it. `pk_s.dll` does that one job.

- It reads `cmd.txt` and answers in `res.txt`. Both files sit in the mod's own folder, next to the DLL.
- For each order, it checks that the memory it is given is readable and holds the expected old values, then writes the new thresholds. If the old values do not match, it writes nothing.
- It imports eight functions from `kernel32.dll` and nothing else: file read and write, memory query and allocation. No network, no registry, no process or thread creation, no other library.
- If it cannot write, Beryl switches its Detail Distance setting off and the rest of the mod keeps working.

The whole source is one file of 60 lines: [pk_s.c](pk_s.c).

## Build it yourself

You need LLVM: `clang`, `lld-link` and `llvm-dlltool`.

```bash
./build_dll.sh
```

The build is reproducible (`/brepro`): the same source gives the same bytes. The file shipped in Beryl 0.1.0-alpha is 8,704 bytes, with this SHA-256:

```
0d8019f7d2175e28b89de984adf89f314a55c0add2792c5ddb0f531c74772e9f
```

Build it, run `sha256sum pk_s.dll`, and compare.

## Comments and bugs

We would love to hear how Beryl works on your machine. Leave a comment, share your experience or report a bug on the mod's page so we can investigate and improve it:

- Nexus Mods: https://www.nexusmods.com/palworld/mods/6089 (Posts and Bugs tabs)
- GitHub: https://github.com/PikouGame/Beryl/issues

## Credits and license

Designed, directed and tested by LeLoraxe. The code was written with OpenAI Codex and Claude. All rights reserved; see [LICENSE.txt](LICENSE.txt).
