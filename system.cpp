#include <cstdint>

extern "C" {
  void printChar(char c) {
    *(volatile uint32_t*)0x4001100C |= (1 << 13) | (1 << 3);

    *(volatile uint32_t*)0x40011004 = c;
  }

  void printH() {
    printChar('H');
  }
}
