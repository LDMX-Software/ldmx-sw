#include "SimCore/G4Session.h"

namespace simcore {

LoggedSession::LoggedSession(std::string logging_prefix)
    : the_log_{::framework::logging::makeLogger(logging_prefix)} {}

G4int LoggedSession::ReceiveG4cout(const G4String& msg) {
  std::string message = msg;
  std::transform(message.begin(), message.end(), message.begin(), ::toupper);

  if (message.find("WARNING") != std::string::npos) {
    ldmx_log(warn) << msg;
  } else if (message.find("ERROR") != std::string::npos) {
    ldmx_log(error) << msg;
  } else {
    ldmx_log(debug) << msg;
  }

  return 0;
}

G4int LoggedSession::ReceiveG4cerr(const G4String& msg) {
  std::string message = msg;
  std::transform(message.begin(), message.end(), message.begin(), ::toupper);

  if (message.find("ERROR") != std::string::npos ||
      message.find("FATAL") != std::string::npos) {
    ldmx_log(error) << msg;
  } else {
    ldmx_log(debug) << msg;
  }

  return 0;
}

}  // namespace simcore
