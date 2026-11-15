#!/bin/bash
# Builds pk_s.dll (x86-64 Windows) from pk_s.c with clang + lld-link, next to this script, or into $1.
# Needs clang, lld-link, llvm-dlltool (LLVM). Imports: kernel32 only. Export: pk_run. /brepro: the same source gives the same bytes.
set -e
H=$(cd "$(dirname "$0")" && pwd)
OUT=${1:-$H/pk_s.dll}
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
cat >"$T/kernel32.def" <<'DEF'
LIBRARY kernel32.dll
EXPORTS
CreateFileA
ReadFile
WriteFile
CloseHandle
VirtualQuery
VirtualAlloc
VirtualFree
GetModuleFileNameA
DEF
llvm-dlltool -m i386:x86-64 -d "$T/kernel32.def" -l "$T/kernel32.lib"
cd "$H"
clang --target=x86_64-pc-windows-msvc -O1 -ffreestanding -fno-builtin -c pk_s.c -o "$T/pk_s.obj" # -O1 gave the first DLL's exact size (7680 bytes)
lld-link /dll /noentry /nodefaultlib /brepro /export:pk_run "$T/pk_s.obj" "$T/kernel32.lib" /out:"$T/pk_s.dll" >/dev/null
mkdir -p "$(dirname "$OUT")";cp "$T/pk_s.dll" "$OUT"
echo "built $OUT ($(stat -c%s "$OUT") bytes, sha256 $(sha256sum "$OUT"|cut -c1-64))"
