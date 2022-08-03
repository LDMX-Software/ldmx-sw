#ifndef SIMCORE_PLUGINFACTORY_H
#define SIMCORE_PLUGINFACTORY_H

#include <variant>

#include "Framework/Configure/Parameters.h"

#include "SimCore/G4User/SteppingAction.h"
#include "SimCore/G4User/EventAction.h"
#include "SimCore/G4User/RunAction.h"
#include "SimCore/G4User/StackingAction.h"
#include "SimCore/G4User/TrackingAction.h"

#include "SimCore/UserAction.h"
#include "SimCore/PrimaryGenerator.h"
#include "SimCore/XsecBiasingOperator.h"
#include "SimCore/SensitiveDetector.h"

namespace simcore {

/**
 * @typedef actionMap
 * A map of the different types of actions to their reference.
 */
typedef std::map<simcore::TYPE,
                 std::variant<simcore::g4user::RunAction*, simcore::g4user::EventAction*,
                              simcore::g4user::TrackingAction*, simcore::g4user::SteppingAction*,
                              simcore::g4user::StackingAction*>>
    actionMap;

/**
 * @class PluginFactory
 * @brief Class that manages the generators used to fire particles.
 *
 * Follows the template for a modern C++ singleton explained
 * <a href="https://stackoverflow.com/a/1008289">on stackoverflow</a> 
 */
class PluginFactory {
 public:
  /// @return the global PluginFactory instance
  static PluginFactory& getInstance();

  /// Delete the copy constructor
  PluginFactory(const PluginFactory&) = delete;

  /// Delete the assignment operator
  void operator=(const PluginFactory&) = delete;

  /**
   * Get the collection of all enabled generators
   *
   * @return vector of pointers to constructed primary generators
   */
  std::vector<simcore::PrimaryGenerator*> getGenerators() const { return generators_; };

  /**
   * Put the primary generator into the list of possible generators
   *
   * @see simcore::PrimaryGenerator::declare for where this method is called.
   * This method is used to construct a list of all possible generators that
   * the user could use.
   *
   * @param[in] className full name of class (including namespaces) of generator
   * @param[in] builder pointer to function to use to create the generator
   */
  void registerGenerator(const std::string& className,
               simcore::PrimaryGeneratorBuilder* builder);

  /**
   * Create a new generate and attach it to the list of generators
   *
   * This checks the list of registered generators for the input className.
   * If the className is not found, then we assume that the generator is not
   * registered and throw and exception.
   *
   * Otherwise, we use the registered builder to create a generator and give
   * it the passed instanceName and paramters. We insert the created generator 
   * into the list of generators.
   *
   * @param[in] className Full name of class (including namespaces) of the generator
   * @param[in] instanceName unique run-time instance name for the generator
   * @param[in] parameters Parameters to pass to the generator for configuration
   */
  void createGenerator(const std::string& className,
                    const std::string& instanceName, const framework::config::Parameters& parameters);

  /**
   * Get the map of all types of user actions to 
   * a pointer to the user action we have created.
   *
   * @note The createAction method assumes that the internal
   * map of actions has already been created, so the user
   * should call this method before any calls to createAction.
   *
   * @return actionMap of created user actions
   */
  actionMap getActions();

  /**
   * Put the user action into the list of possible actions.
   *
   * @see simcore::UserAction::declare for where this method is called.
   * This method is used to construct a list of all possible actions that
   * the user could use.
   *
   * @param[in] className full name of class (including namespaces) of action
   * @param[in] builder pointer to function to use to create the action
   */
  void registerAction(const std::string& className, simcore::UserActionBuilder* builder);

  /**
   * Construct a new action and attach it to the types of actions it will be a part of.
   *
   * This checks the list of registered actions for the input className.
   * If the className is not found, then we assume that the action is not
   * registered and throw and exception.
   *
   * Otherwise, we use the registered builder to create a action and give
   * it the passed instanceName and paramters. After creation, we then use
   * UserAction::getTypes() to determine which types of actions we should
   * attach this specific action to.
   *
   * @param[in] className Full name of class (including namespaces) of the action
   * @param[in] instanceName unique run-time instance name for the action
   * @param[in] parameters Parameters to pass to the action for configuration
   */
  void createAction(const std::string& className,
                    const std::string& instanceName, framework::config::Parameters& parameters);

  /**
   * Retrieve the current list of biasing operators.
   *
   * In un-biased running modes, this will return an empty vector.
   *
   * @return vector of pointers to biasing operators
   */
  std::vector<XsecBiasingOperator*> getBiasingOperators() const { return biasing_operators_; }

  /**
   * Put the biasing operator into the list of possible biasing operators.
   *
   * @see simcore::XsecBiasingOperator::declare for where this method is called.
   * The declare method is then called using the DECLARE_XSECBIASINGOPERATOR macro.
   *
   * @param[in] className Full name of class (including namespaces) of the operator
   * @param[in] builder a pointer to the function that should be used to create the operator
   */
  void registerBiasingOperator(const std::string& className, XsecBiasingOperatorBuilder* builder);

  /**
   * Create a biasing operator from the input parameters.
   *
   * This checks the list of registered biasing operators for the input className.
   * If the className is not found, then we assume that the biasing operator is not
   * registered and throw and exception.
   *
   * Otherwise, we use the registered builder to create an operator and give
   * it the passed instanceName and paramters. We insert the created operator
   * into the list of biasing operators.
   *
   * @param[in] classNmae Full name of class (including namespaces) of the operator
   * @param[in] instanceName unique run-time instance name for the operator
   * @param[in] parameters Parameters to pass to the operator for configuration
   */
  void createBiasingOperator(const std::string& className,
                    const std::string& instanceName, const framework::config::Parameters& parameters);

  /**
   * Retrieve the current list of created sensitive detectors.
   *
   * @return vector of pointers to sensitive detectors
   */
  std::vector<SensitiveDetector*> getSensitiveDetectors() const { return sensitive_detectors_; }

  /**
   * Put the sensitive detector into the list of possible sensitive detectors.
   *
   * @see simcore::SensitiveDetector::declare for where this method is called.
   * The declare method is then called using the DECLARE_SENSITIVEDETECTOR macro.
   *
   * @param[in] className Full name of class (including namespaces) of the detector
   * @param[in] builder a pointer to the function that should be used to create the detector
   */
  void registerSensitiveDetector(const std::string& className, SensitiveDetectorBuilder* builder);

  /**
   * Create a sensitive detector from the input parameters.
   *
   * This checks the list of registered sensitive detectors for the input className.
   * If the className is not found, then we assume that the sensitive detector is not
   * registered and throw and exception.
   *
   * Otherwise, we use the registered builder to create an operator and give
   * it the passed instanceName and paramters. Then we store the pointer to
   * this object in our list of sensitive detectors.
   *
   * @note The G4SDManager owns and cleans up the sensitive detectors that are
   * registered with it. We register all sensitive detectors in the constructor.
   *
   * @param[in] classNmae Full name of class (including namespaces) of the detector
   * @param[in] instanceName unique run-time instance name for the detector
   * @param[in] ci handle to conditions interface to pass to the detector
   * @param[in] parameters Parameters to pass to the detector for configuration
   */
  void createSensitiveDetector(const std::string& className, const std::string& instanceName, 
      simcore::ConditionsInterface& ci, const framework::config::Parameters& parameters);

 private:
  /// Constructor - private to prevent initialization
  PluginFactory() {}

  /// A map of all register generators to their builders
  std::map<std::string, simcore::PrimaryGeneratorBuilder*> registeredGenerators_;

  /// Cointainer for all generators to be used by the simulation
  std::vector<simcore::PrimaryGenerator*> generators_;

  /// A map of all registered user actions to their corresponding info.
  std::map<std::string, simcore::UserActionBuilder*> registeredActions_;

  /// Container for all Geant4 actions
  actionMap actions_;

  /// A map of all registered user actions to their corresponding info.
  std::map<std::string, simcore::XsecBiasingOperatorBuilder*> registeredOperators_;

  /// Container for all biasing operators
  std::vector<simcore::XsecBiasingOperator*> biasing_operators_;

  /// A map of all registered sensitive detectors
  std::map<std::string, simcore::SensitiveDetectorBuilder*> registeredDetectors_;

  /// Container of all created sensitive detectors
  std::vector<simcore::SensitiveDetector*> sensitive_detectors_;

};  // PluginFactory

}  // namespace simcore

#endif  // SIMCORE_PLUGINFACTORY_H
