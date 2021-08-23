#include "Trigger/Event/TrigEnergySum.h"

#include <iostream>

ClassImp(ldmx::TrigEnergySum)

namespace ldmx {
  
  TrigEnergySum::TrigEnergySum(int layer, int hwEnergy)
      : layer_(layer), hwEnergy_{hwEnergy} {}

  void TrigEnergySum::Print() const { std::cout << *this << std::endl; }

  std::ostream &operator<<(std::ostream &s, const ldmx::TrigEnergySum &sum) {
    s << "TrigEnergySum { "
      << "(layer " << sum.layer() << ", hwEnergy " << sum.hwEnergy() << " } ";
    return s;
  }

  std::ostream &operator<<(std::ostream &s,
                           const ldmx::TrigEnergySumCollection &sums) {
    s << "TrigEnergySumCollection { " << std::endl;
    for (auto sum : sums) s << "  " << sum << std::endl;
    s << "}";
    return s;
  }

}  // namespace ldmx



