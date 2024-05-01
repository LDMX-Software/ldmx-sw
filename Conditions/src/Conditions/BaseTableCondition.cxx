#include <algorithm>
#include "Conditions/SimpleTableCondition.h"

std::ostream& operator<<(std::ostream& s, const conditions::BaseTableCondition& btc) {
  s<<"[ TableCondition: " << btc.getName() << std::endl;
  s<<"  DetID";
  const std::vector<std::string>& names=btc.getColumnNames();
  for (auto name: names) {
    s<< ",id:\"" << name <<'"';
  }
  s<<std::endl;
  for (std::size_t i=0; i<btc.getRowCount(); i++) {
    s<<"  ";
    btc.streamRow(s,i);
  }  
  return s<<']';
}


namespace conditions {

std::size_t BaseTableCondition::findKey(unsigned int id) const {
  unsigned int effid = id & idMask_;
  std::vector<unsigned int>::const_iterator ptr =
      std::lower_bound(keys_.begin(), keys_.end(), effid);
  if (ptr == keys_.end() || *ptr != effid)
    return keys_.size();
  else
    return std::distance(keys_.begin(), ptr);
}

std::size_t BaseTableCondition::findKeyInsert(unsigned int id) const {
  unsigned int effid = id & idMask_;
  std::vector<unsigned int>::const_iterator ptr =
      std::lower_bound(keys_.begin(), keys_.end(), effid);
  return std::distance(keys_.begin(), ptr);
}

}  // namespace conditions
