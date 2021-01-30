#include "Recon/Event/HgcrocTrigDigi.h"

#include <iostream>

ClassImp(ldmx::HgcrocTrigDigi)

    namespace ldmx {
  HgcrocTrigDigi::HgcrocTrigDigi(uint32_t tid, uint8_t tp)
      : tid_(tid), tp_{tp} {}

  void HgcrocTrigDigi::Print() const { std::cout << *this << std::endl; }

  uint8_t HgcrocTrigDigi::linear2Compressed(uint32_t lin) {
    if (lin >= 0x40000) return 0x7F;  // saturation
    if (lin >= 0x20000) return 0x78 | ((lin >> 14) & 0x7);
    if (lin >= 0x10000) return 0x70 | ((lin >> 13) & 0x7);
    if (lin >= 0x8000) return 0x68 | ((lin >> 12) & 0x7);
    if (lin >= 0x4000) return 0x60 | ((lin >> 11) & 0x7);
    if (lin >= 0x2000) return 0x58 | ((lin >> 10) & 0x7);
    if (lin >= 0x1000) return 0x50 | ((lin >> 9) & 0x7);
    if (lin >= 0x800) return 0x48 | ((lin >> 8) & 0x7);
    if (lin >= 0x400) return 0x40 | ((lin >> 7) & 0x7);
    if (lin >= 0x200) return 0x38 | ((lin >> 6) & 0x7);
    if (lin >= 0x100) return 0x30 | ((lin >> 5) & 0x7);
    if (lin >= 0x80) return 0x28 | ((lin >> 4) & 0x7);
    if (lin >= 0x40) return 0x20 | ((lin >> 3) & 0x7);
    if (lin >= 0x20) return 0x18 | ((lin >> 2) & 0x7);
    if (lin >= 0x10) return 0x10 | ((lin >> 1) & 0x7);
    if (lin >= 0x08)
      return 0x08 | ((lin >> 0) & 0x7);
    else
      return lin & 0x7;
  }

  uint32_t HgcrocTrigDigi::compressed2Linear(uint8_t comp) {
    uint32_t v1 = ((comp & 0x78) == 0)
                      ? (comp)
                      : ((0x8 | (comp & 0x7)) << ((comp >> 3) - 1));
    uint8_t comp2 = comp + 1;
    uint32_t v2 = ((comp2 & 0x78) == 0)
                      ? (comp2)
                      : ((0x8 | (comp2 & 0x7)) << ((comp2 >> 3) - 1));
    return (v1 + v2) / 2;
  }

  std::ostream &operator<<(std::ostream &s, const ldmx::HgcrocTrigDigi &digi) {
    s << "HgcrocTrigDigi { "
      << "(id : 0x" << std::hex << digi.getId() << std::dec << ") ";
    s << "0x" << std::hex << int(digi.getPrimitive()) << " (" << std::dec
      << digi.linearPrimitive() << ") } ";
    return s;
  }

  std::ostream &operator<<(std::ostream &s,
                           const ldmx::HgcrocTrigDigiCollection &digis) {
    s << "HgcrocTrigDigiCollection { " << std::endl;
    for (auto digi : digis) s << "  " << digi << std::endl;
    s << "}";
    return s;
  }

}  // namespace ldmx
