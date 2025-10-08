#include "Trigger/Event/TrigEnergySum.h"

#include <iostream>

ClassImp(trigger::TrigEnergySum);

namespace trigger {
TrigEnergySum::TrigEnergySum(int layer, int hwEnergy)
    : layer_{layer}, module_{0}, hw_energy_{hwEnergy}, energy_{0} {}
TrigEnergySum::TrigEnergySum(int layer, int module, float energy)
    : layer_{layer}, module_{module}, hw_energy_{0}, energy_{energy} {}

std::ostream &operator<<(std::ostream &s, const trigger::TrigEnergySum &sum) {
  s << "TrigEnergySum { " << "(layer " << sum.layer() << ", hwEnergy "
    << sum.hwEnergy() << " } ";
  return s;
}

std::ostream &operator<<(std::ostream &s,
                         const trigger::TrigEnergySumCollection &sums) {
  s << "TrigEnergySumCollection { " << std::endl;
  for (auto sum : sums) s << "  " << sum << std::endl;
  s << "}";
  return s;
}

}  // namespace trigger
