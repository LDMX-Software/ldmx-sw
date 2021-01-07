
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
  auto tree{new TTree{name.c_str(), name.c_str()}};
  trees_[name] = tree;
}

void NtupleManager::fill() {
  // Make sure there are trees to fill
  if (trees_.empty()) return;

  // Loop over all the trees and fill them
  for (const auto& element : trees_) {
    element.second->Fill();
  }
}

void NtupleManager::clear() {
  // Make sure there are variables to clear.
  if (variables_.empty()) return;

  // Loop over all of the variables and set them to a default.
  for (const auto& element : variables_) {
    variables_[element.first] = -9999;
  }
}
}  // namespace framework
