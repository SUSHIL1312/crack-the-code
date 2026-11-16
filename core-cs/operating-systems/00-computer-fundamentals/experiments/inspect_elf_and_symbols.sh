#!/usr/bin/env bash
# ==============================================================================
# inspect_elf_and_symbols.sh
# Hands-on exploration script to inspect binary headers, symbol tables,
# and section sizes across Linux and macOS.
# ==============================================================================

set -euo pipefail

TARGET_CPP="../examples/inspect_memory_layout.cpp"
OUTPUT_BIN="/tmp/test_inspect_bin"

echo "=== 1. Compiling Target Binary ==="
clang++ -std=c++20 -O0 "$TARGET_CPP" -o "$OUTPUT_BIN"
echo "Compiled successfully to $OUTPUT_BIN"
echo

echo "=== 2. Section Sizes (Text, Data, BSS) via 'size' ==="
size "$OUTPUT_BIN"
echo "Note: .text = executable code, .data = initialized globals, .bss = uninitialized globals."
echo

echo "=== 3. Symbol Table Inspection via 'nm' ==="
echo "Filtering for global symbols defined in our target source:"
if command -v nm >/dev/null 2>&1; then
    nm "$OUTPUT_BIN" | grep -E "(g_initialized_var|g_uninitialized_var|dummy_function)" || true
fi
echo

echo "=== 4. Binary Header Inspection ==="
if command -v readelf >/dev/null 2>&1; then
    echo "Using readelf (Linux ELF):"
    readelf -h "$OUTPUT_BIN" | head -n 15
elif command -v otool >/dev/null 2>&1; then
    echo "Using otool / objdump (macOS Mach-O):"
    otool -hv "$OUTPUT_BIN"
fi
echo

echo "=== 5. How to Trace System Calls on Linux ==="
echo "On Linux systems, run:"
echo "  strace -c $OUTPUT_BIN"
echo "This provides a statistical summary of all system calls (openat, mmap, write, etc.),"
echo "their total execution time, and call frequency."
echo "=============================================================================="
