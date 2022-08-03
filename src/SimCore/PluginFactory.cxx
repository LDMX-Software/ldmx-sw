
#include "SimCore/PluginFactory.h"

/*~~~~~~~~~~~~~~~~*/
/*   C++ StdLib   */
/*~~~~~~~~~~~~~~~~*/
#include <dlfcn.h>
#include <algorithm>
#include <string>
#include <vector>

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Exception/Exception.h"

namespace simcore {

PluginFactory& PluginFactory::getInstance() { 
  //the_factory is created on first call to getInstance
  //  and is guaranteed to be destroyed
  static PluginFactory the_factory;
  return the_factory; 
}

void PluginFactory::registerGenerator(const std::string& className,
                                      simcore::PrimaryGeneratorBuilder* builder) {

  if (registeredGenerators_.find(className) != registeredGenerators_.end()) {
    EXCEPTION_RAISE(
        "ExistingGeneratorDefinition",
        "The primary generator " + className + " has already been registered.");
  }

  registeredGenerators_[className] = builder;
}

void PluginFactory::createGenerator(const std::string& className,
                                    const std::string& instanceName,
                                    const framework::config::Parameters& parameters) {
  if (registeredGenerators_.find(className) == registeredGenerators_.end()) {
    EXCEPTION_RAISE("CreateGenerator",
                    "Failed to create generator '" + className + "'.");
  }

  auto generator{registeredGenerators_[className](instanceName, parameters)};

  // now that the generator is built --> put it on active list
  generators_.push_back(generator);
}

actionMap PluginFactory::getActions() {
  if (actions_.empty()) {
    actions_[simcore::TYPE::RUN] = new simcore::g4user::RunAction();
    actions_[simcore::TYPE::EVENT] = new simcore::g4user::EventAction();
    actions_[simcore::TYPE::TRACKING] = new simcore::g4user::TrackingAction();
    actions_[simcore::TYPE::STEPPING] = new simcore::g4user::SteppingAction();
    actions_[simcore::TYPE::STACKING] = new simcore::g4user::StackingAction();
  }

  return actions_;
}

void PluginFactory::registerAction(const std::string& className,
                                   simcore::UserActionBuilder* builder) {
  if (registeredActions_.find(className) != registeredActions_.end()) {
    EXCEPTION_RAISE(
        "ExistingActionDefinition",
        "The user action " + className + " has already been registered.");
  }

  registeredActions_[className] = builder;
}

void PluginFactory::createAction(const std::string& className,
                                 const std::string& instanceName,
                                 framework::config::Parameters& parameters) {
  if (registeredActions_.find(className) == registeredActions_.end()) {
    EXCEPTION_RAISE("PluginFactory", "Failed to create " + className);
  }

  auto act{registeredActions_[className](instanceName, parameters)};

  std::vector<simcore::TYPE> types = act->getTypes();
  for (auto& type : types) {
    if (type == simcore::TYPE::RUN)
      std::get<simcore::g4user::RunAction*>(actions_[simcore::TYPE::RUN])->registerAction(act);
    else if (type == simcore::TYPE::EVENT)
      std::get<simcore::g4user::EventAction*>(actions_[simcore::TYPE::EVENT])->registerAction(act);
    else if (type == simcore::TYPE::TRACKING)
      std::get<simcore::g4user::TrackingAction*>(actions_[simcore::TYPE::TRACKING])
          ->registerAction(act);
    else if (type == simcore::TYPE::STEPPING)
      std::get<simcore::g4user::SteppingAction*>(actions_[simcore::TYPE::STEPPING])->registerAction(act);
    else if (type == simcore::TYPE::STACKING)
      std::get<simcore::g4user::StackingAction*>(actions_[simcore::TYPE::STACKING])
          ->registerAction(act);
    else
      EXCEPTION_RAISE("PluginFactory", "User action type doesn't exist.");
  }
}

void PluginFactory::registerBiasingOperator(
    const std::string& className, XsecBiasingOperatorBuilder* builder) {

  if (registeredOperators_.find(className) != registeredOperators_.end()) {
    EXCEPTION_RAISE(
        "ExistingOperatorDefinition",
        "The biasing operator " + className + " has already been registered.");
  }

  registeredOperators_[className] = builder;
}

void PluginFactory::createBiasingOperator(const std::string& className,
                                          const std::string& instanceName,
                                          const framework::config::Parameters& parameters) {
  if (registeredOperators_.find(className) == registeredOperators_.end()) {
    EXCEPTION_RAISE("CreateBiasingOperator",
                    "Failed to create biasing '" + className + "'.");
  }

  auto bop{registeredOperators_[className](instanceName, parameters)};

  // now that the biasing is built --> put it on active list
  std::cout << "[ PluginFactory ]: Biasing operator '"
    << instanceName << "' of class '"
    << className << "' has been created." << std::endl;
  biasing_operators_.push_back(bop);
}

void PluginFactory::registerSensitiveDetector(
    const std::string& className, SensitiveDetectorBuilder* builder) {

  if (registeredDetectors_.find(className) != registeredDetectors_.end()) {
    EXCEPTION_RAISE(
        "ExistingOperatorDefinition",
        "The biasing operator " + className + " has already been registered.");
  }

  registeredDetectors_[className] = builder;
}

void PluginFactory::createSensitiveDetector(const std::string& className,
                                            const std::string& instanceName,
                                            simcore::ConditionsInterface& ci,
                                            const framework::config::Parameters& parameters) {
  if (registeredDetectors_.find(className) == registeredDetectors_.end()) {
    EXCEPTION_RAISE("CreateSensitiveDetector",
                    "Failed to create detector '" + className + "'.");
  }

  auto det{registeredDetectors_[className](instanceName, ci, parameters)};

  sensitive_detectors_.push_back(det);
}

}  // namespace simcore
