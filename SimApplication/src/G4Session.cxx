/**
 * @file G4Session.cxx
 * @brief Classes which redirect the output of G4cout and G4cerr
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "SimApplication/G4Session.h"

#include "Exception/Exception.h"

namespace ldmx {

    G4int LoggedSession::ReceiveG4cout(const G4String& message) {
        //attempt to remove new line character because log will add another one
        //  the code after the << will _only be executed_ if debug log is being written
        ldmx_log(debug) << G4String(message).strip(G4String::stripType::trailing,'\n').data();
        return 0; //0 return value == sucess
    }

    G4int LoggedSession::ReceiveG4cerr(const G4String& message) {
        //attempt to remove new line character because log will add another one
        //  the code after the << will _only be executed_ if debug log is being written
        ldmx_log(info) << G4String(message).strip(G4String::stripType::trailing,'\n').data();
        return 0; //0 return value == sucess
    }
}
