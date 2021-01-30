/**
 * @file G4Session.h
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef SIMCORE_G4SESSION_H_
#define SIMCORE_G4SESSION_H_

#include <fstream>
#include <iostream>

// Geant4
#include "G4UIsession.hh"

namespace simcore {

/**
 * @class LoggedSession
 *
 * Log the output of Geant4 to files in current directory.
 */
class LoggedSession : public G4UIsession {
 public:
  /**
   * Constructor
   *
   * Sets up output file streams for the cout and cerr paths.
   */
  LoggedSession(const std::string& coutFileName = "G4cout.log",
                const std::string& cerrFileName = "G4cerr.log");

  /**
   * Destructor
   *
   * Closes the output files streams
   */
  ~LoggedSession();

  /**
   * Required hook for Geant4
   *
   * Does nothing
   */
  G4UIsession* SessionStart() { return nullptr; }

  /**
   * Redirects cout to file
   */
  G4int ReceiveG4cout(const G4String& message);

  /**
   * Redirects cerr to file
   */
  G4int ReceiveG4cerr(const G4String& message);

 private:
  /** cout log file */
  std::ofstream coutFile_;

  /** cerr log file */
  std::ofstream cerrFile_;

};  // LoggedSession

/**
 * @class BatchSession
 *
 * Do _nothing_ with G4cout and G4cerr messages. This is made to improve
 * performance.
 */
class BatchSession : public G4UIsession {
 public:
  /**
   * Constructor
   */
  BatchSession() {}

  /**
   * Destructor
   */
  ~BatchSession() {}

  /**
   * Required hook for Geant4
   *
   * Does nothing
   */
  G4UIsession* SessionStart() { return nullptr; }

  /**
   * Does nothing with input
   */
  G4int ReceiveG4cout(const G4String&) { return 0; }

  /**
   * Does nothing with input
   */
  G4int ReceiveG4cerr(const G4String&) { return 0; }
};

}  // namespace simcore

#endif
