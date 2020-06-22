/**
 * @file Logger.cxx
 * Interface for ldmx-sw to spdlog
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Exception/Logger.h"

namespace ldmx {

    void initLog(int termV, int fileV) {

        if ( fileV > 0 ) {
            logging::add_file_log(
                    keywords::file_name = "ldmx_app.log",
                    keywords::format = "[%TimeStamp%] : %Message%"
                    );
            //set verbosity for file log?
        }

        if ( termV > 0 ) {
            //set verbosity for terminal sink?
        }

        //TODO make severity setting depend on verbosities
        logging::core::get()->set_filter(
                logging::trivial::severity >= logging::trivial::info
                );

    }

} //ldmx
