#include "Recon/Event/CaloTrigPrim.h"

#include <iostream>

ClassImp(ldmx::CaloTrigPrim)

    namespace ldmx {
  CaloTrigPrim::CaloTrigPrim(uint32_t tid, uint32_t tp) : tid_(tid), tp_{tp} {}

  void CaloTrigPrim::Print() const { std::cout << *this << std::endl; }

  std::ostream &operator<<(std::ostream &s, const ldmx::CaloTrigPrim &digi) {
    s << "CaloTrigPrim { "
      << "(id : 0x" << std::hex << digi.getId() << std::dec << ") ";
    s << "0x" << std::hex << int(digi.getPrimitive()) << " } ";
    return s;
  }

  std::ostream &operator<<(std::ostream &s,
                           const ldmx::CaloTrigPrimCollection &digis) {
    s << "CaloTrigPrimCollection { " << std::endl;
    for (auto digi : digis) s << "  " << digi << std::endl;
    s << "}";
    return s;
  }

}  // namespace ldmx
