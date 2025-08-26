/**
 * @file G4Session.cxx
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimCore/G4Session.h"

#include "Framework/Exception/Exception.h"

namespace simcore {

LoggedSession::LoggedSession(const std::string& coutFileName,
                             const std::string& cerrFileName) {
  cout_file_.open(coutFileName);
  if (not cout_file_.is_open()) {
    EXCEPTION_RAISE("G4Logging",
                    "Unable to open log file '" + coutFileName + "'.");
  }

  cerr_file_.open(cerrFileName);
  if (not cerr_file_.is_open()) {
    EXCEPTION_RAISE("G4Logging",
                    "Unable to open log file '" + cerrFileName + "'.");
  }
}

LoggedSession::~LoggedSession() {
  cout_file_.close();
  cerr_file_.close();
}

G4int LoggedSession::ReceiveG4cout(const G4String& message) {
  cout_file_ << message;
  cout_file_.flush();
  return 0;  // 0 return value == sucess
}

G4int LoggedSession::ReceiveG4cerr(const G4String& message) {
  cerr_file_ << message;
  cerr_file_.flush();
  return 0;  // 0 return value == sucess
}
}  // namespace simcore
