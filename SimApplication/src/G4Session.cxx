/**
 * @file G4Session.cxx
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/G4Session.h"

#include "Exception/Exception.h"

namespace ldmx {

    G4int LoggedSession::ReceiveG4cout(const G4String& message) {
        ldmx_log(debug) << message.data();
        return 0; //0 return value == sucess
    }

    G4int LoggedSession::ReceiveG4cerr(const G4String& message) {
        ldmx_log(info) << message.data();
        return 0; //0 return value == sucess
    }
}
