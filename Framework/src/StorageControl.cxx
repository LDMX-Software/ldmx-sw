#include "Framework/StorageControl.h"

namespace ldmx {

  void StorageControl::resetEventState() {
    hints_.clear();
  }

  void StorageControl::addHint(const std::string& processor_name, ldmx::StorageControlHint hint, const std::string& purposeString) {
    hints_.push_back(Hint());
    hints_.back().evpName_=processor_name;
    hints_.back().hint_=hint;
    hints_.back().purpose_=purposeString;
  }
  
  bool StorageControl::keepEvent() const {
    return defaultIsKeep_;
  }
  
}
