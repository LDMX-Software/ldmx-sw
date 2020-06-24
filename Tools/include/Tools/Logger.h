#ifndef TOOLS_LOGGER_H
#define TOOLS_LOGGER_H

/**
 * Necessary to get linking to work?
 * https://stackoverflow.com/questions/23137637/linker-error-while-linking-boost-log-tutorial-undefined-references
 * https://www.boost.org/doc/libs/1_54_0/libs/log/doc/html/log/rationale/namespace_mangling.html
 * https://stackoverflow.com/a/40016057
 */
#define BOOST_ALL_DYN_LINK 1

#include <boost/log/core.hpp> //core logging service
#include <boost/log/sinks/sync_frontend.hpp> //syncronous sink frontend
#include <boost/log/sinks/text_ostream_backend.hpp> //output stream sink backend
#include <boost/log/sources/severity_feature.hpp> //for the severity feature in a logger
#include <boost/log/sources/severity_channel_logger.hpp> //for the severity logger
#include <boost/log/sources/global_logger_storage.hpp> //for global logger default
#include <boost/log/expressions.hpp> //for attributes and expressions
#include <boost/log/utility/setup/common_attributes.hpp>

//TODO check which headers are required
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace ldmx {

    /**
     * Severity/Logging levels
     *
     * Out in this namespace so its easier for everyone to use.
     */
    enum level {
        debug = 0, //0
        info,  //1
        warn,  //2 -> default
        error, //3
        fatal  //4 
    };

    namespace logging {

        /**
         * Short names for boost namespaces
         */
        namespace log = boost::log;
        namespace sinks = boost::log::sinks;


        /**
         * Define the type of logger we will be using in ldmx-sw
         */
        typedef log::sources::severity_channel_logger_mt< level, std::string > logger;

        /**
         * Gets a logger for the user
         *
         * Returns a logger type with some extra initialization procedures done.
         * Should _only be called ONCE_ during a run.
         *
         * @param name name of this logging channel (e.g. processor name)
         * @return logger with the input channel name
         */
        logger makeLogger( const std::string& name );
    
        /**
         * Initialize the logging backend
         *
         * This function setups up the terminal and file sinks.
         * Sets their format and filtering level for this run.
         *
         * @param loggingArgs list of command line parameters passed
         */
        void open(const std::vector<std::string>& loggingArgs);

        /**
         * Close up the logging
         */
        void close();

    } //logging

} //ldmx

#endif //TOOLS_LOGGER_H
