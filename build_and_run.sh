#!/usr/bin/env bash
set -e

# Collect all Assembly and C++ source files in current dir
ASM_SOURCES=$(find . -maxdepth 1 -name "*.s")
CPP_SOURCES=$(find . -maxdepth 1 -name "*.cpp")

if [ -z "$ASM_SOURCES" ] && [ -z "$CPP_SOURCES" ]; then
    echo "❌ No .s or .cpp files found in current directory!"
    exit 1
fi

mkdir -p build

echo "🔨 Compiling bare-metal targets..."

arm-none-eabi-g++ \
    -mcpu=cortex-m4 \
    -mthumb \
    -O0 -g \
    -fno-exceptions \
    -fno-rtti \
    -fno-threadsafe-statics \
    -nostdlib \
    -T linker.ld \
    $ASM_SOURCES $CPP_SOURCES \
    -o build/firmware.elf

echo "✅ Generated build/firmware.elf"
echo "🚀 Spawning Renode..."

renode board.resc
