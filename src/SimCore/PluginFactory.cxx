
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
  // the_factory is created on first call to getInstance
  //  and is guaranteed to be destroyed
  static PluginFactory the_factory;
  return the_factory;
}

void PluginFactory::registerGenerator(const std::string& className,
                                      PrimaryGeneratorBuilder* builder) {
  auto it{registeredGenerators_.find(className)};
  if (it != registeredGenerators_.end()) {
    EXCEPTION_RAISE(
        "ExistingGeneratorDefinition",
        "The primary generator " + className + " has already been registered.");
  }

  registeredGenerators_[className] = builder;
}

void PluginFactory::createGenerator(const std::string& className,
                                    const std::string& instanceName,
                                    framework::config::Parameters& parameters) {
  auto it{registeredGenerators_.find(className)};
  if (it == registeredGenerators_.end()) {
    EXCEPTION_RAISE("CreateGenerator",
                    "Failed to create generator '" + className + "'.");
  }

  auto generator{it->second(instanceName, parameters)};

  // now that the generator is built --> put it on active list
  generators_.push_back(generator);

  ldmx_log(info) << "Primary Generator '" << instanceName << "' of class '"
                 << className << "' has been created.";
}

actionMap PluginFactory::getActions() {
  if (actions_.empty()) {
    actions_[TYPE::RUN] = new UserRunAction();
    actions_[TYPE::EVENT] = new UserEventAction();
    actions_[TYPE::TRACKING] = new UserTrackingAction();
    actions_[TYPE::STEPPING] = new USteppingAction();
    actions_[TYPE::STACKING] = new UserStackingAction();
  }

  return actions_;
}

void PluginFactory::registerAction(const std::string& className,
                                   UserActionBuilder* builder) {
  auto it{registeredActions_.find(className)};
  if (it != registeredActions_.end()) {
    EXCEPTION_RAISE(
        "ExistingActionDefinition",
        "The user action " + className + " has already been registered.");
  }

  registeredActions_[className] = builder;
}

void PluginFactory::createAction(const std::string& className,
                                 const std::string& instanceName,
                                 framework::config::Parameters& parameters) {
  auto it{registeredActions_.find(className)};
  if (it == registeredActions_.end()) {
    EXCEPTION_RAISE("PluginFactory", "Failed to create " + className);
  }

  auto act{it->second(instanceName, parameters)};

  std::vector<TYPE> types = act->getTypes();
  for (auto& type : types) {
    if (type == TYPE::RUN)
      std::get<UserRunAction*>(actions_[TYPE::RUN])->registerAction(act);
    else if (type == TYPE::EVENT)
      std::get<UserEventAction*>(actions_[TYPE::EVENT])->registerAction(act);
    else if (type == TYPE::TRACKING)
      std::get<UserTrackingAction*>(actions_[TYPE::TRACKING])
          ->registerAction(act);
    else if (type == TYPE::STEPPING)
      std::get<USteppingAction*>(actions_[TYPE::STEPPING])->registerAction(act);
    else if (type == TYPE::STACKING)
      std::get<UserStackingAction*>(actions_[TYPE::STACKING])
          ->registerAction(act);
    else
      EXCEPTION_RAISE("PluginFactory", "User action type doesn't exist.");
  }

  // now that the biasing is built --> put it on active list
  ldmx_log(info) << "User Action '" << instanceName << "' of class '"
                 << className << "' has been created.";
}

void PluginFactory::registerBiasingOperator(
    const std::string& className, XsecBiasingOperatorBuilder* builder) {
  auto it{registeredOperators_.find(className)};
  if (it != registeredOperators_.end()) {
    EXCEPTION_RAISE(
        "ExistingOperatorDefinition",
        "The biasing operator " + className + " has already been registered.");
  }

  registeredOperators_[className] = builder;
}

void PluginFactory::createBiasingOperator(
    const std::string& className, const std::string& instanceName,
    framework::config::Parameters& parameters) {
  auto it{registeredOperators_.find(className)};
  if (it == registeredOperators_.end()) {
    EXCEPTION_RAISE("CreateBiasingOperator",
                    "Failed to create biasing '" + className + "'.");
  }

  auto bop{it->second(instanceName, parameters)};

  // now that the biasing is built --> put it on active list
  biasing_operators_.push_back(bop);
  ldmx_log(info) << "Biasing operator '" << instanceName << "' of class '"
                 << className << "' has been created.";
}

}  // namespace simcore
