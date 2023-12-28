
#include "Framework/NtupleManager.h"

namespace framework {

NtupleManager::NtupleManager() {}

NtupleManager& NtupleManager::getInstance() {
  // Create an instance of the NtupleManager if needed
  static NtupleManager instance;
  return instance;
}

void NtupleManager::create(const std::string& name) {
  // Check if a tree named 'name' has already been created.  If so
  // throw an exception.
  if (trees_.count(name) != 0)
    EXCEPTION_RAISE("NtupleManager",
                    "A tree with name " + name + " has already been created.");

  // Create a tree with the given name and add it to the list of trees.
  trees_[name] = new TTree{name.c_str(), name.c_str()};
}

void NtupleManager::fill() {
  // Loop over all the trees and fill them
  for (const auto& [name, tree] : trees_) tree->Fill();
}

void NtupleManager::clear() { bus_.clear(); }

void NtupleManager::reset() {
  // we assume that ROOT handles clean-up
  //  of the TTrees when they are written to the output histogram file
  trees_.clear();
  bus_.everybodyOff();
}

}  // namespace framework
