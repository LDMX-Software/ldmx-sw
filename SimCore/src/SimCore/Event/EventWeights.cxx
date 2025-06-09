//
// Created by Wesley Ketchum on 4/28/24.
//

#include "SimCore/Event/EventWeights.h"

#include <iostream>

namespace ldmx {

std::map<EventWeights::VariationType, double>
    EventWeights::getVariationsNthWeight(size_t i_w) const
{
  std::map<EventWeights::VariationType, double> i_weights_map;
  for (auto const& v : this->variations_map_)
    i_weights_map[v.first] = v.second.at(i_w);
  return i_weights_map;
}

void EventWeights::Clear() {
  weights_.clear();
  variations_map_.clear();
}

std::ostream &operator<<(std::ostream &o, const EventWeights &ew) {
  o << "Num weights: " << ew.getNumWeights() << std::endl;
  for (size_t i_w = 0; i_w < ew.getNumWeights(); ++i_w) {
    o << "\t" << i_w << ": weight=" << ew.getNthWeight(i_w);
    for (auto const &v : ew.getVariationsNthWeight(i_w)) {
      o << "\n\t var_type=" << v.first << ", var_value=" << v.second;
    }
    o << std::endl;
  }
}

void EventWeights::Print() { std::cout << *this; }

}  // namespace ldmx