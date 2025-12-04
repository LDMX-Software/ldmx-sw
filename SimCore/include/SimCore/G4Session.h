/**
 * @file G4Session.h
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_G4SESSION_H
#define SIMCORE_G4SESSION_H

#include <algorithm>
#include <string>

#include "Framework/Logger.h"
#include "G4UIsession.hh"

namespace simcore {

/**
 * @class LoggedSession
 * @brief Session that routes G4cout and G4cerr through the Framework logger
 *
 * This session intercepts all Geant4 output and routes it through the
 * Framework's logging system with the logger name "Geant4". Messages are
 * parsed to determine appropriate log levels:
 * - G4cout with "WARNING" -> warn
 * - G4cout with "ERROR" -> error
 * - G4cout with verbose tracking info -> trace
 * - G4cout default -> debug
 * - G4cerr with "ERROR" or "FATAL" -> error
 * - G4cerr default -> warn
 */
class LoggedSession : public G4UIsession {
 public:
  /**
   * Constructor - creates a logger named "Geant4"
   */
  LoggedSession(std::string logging_prefix = "Geant4");

  /**
   * Destructor
   */
  ~LoggedSession() override = default;

  /**
   * Receive a message from G4cout
   * @param message The message from Geant4
   * @return 0 for success
   */
  G4int ReceiveG4cout(const G4String& message) override;

  /**
   * Receive a message from G4cerr
   * @param message The message from Geant4
   * @return 0 for success
   */
  G4int ReceiveG4cerr(const G4String& message) override;

 private:
  /// Framework logger for Geant4 messages
  mutable framework::logging::logger the_log_;
};

}  // namespace simcore

#endif  // SIMCORE_G4SESSION_H