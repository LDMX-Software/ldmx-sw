/**
 * @file Logger.h
 * Interface for ldmx-sw to boost logging
 *
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EXCEPTION_LOGGER_H
#define EXCEPTION_LOGGER_H

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
#include <boost/log/trivial.hpp> //for trivial logging levels
#include <boost/log/sources/logger.hpp> //for our logger type
#include <boost/log/sources/global_logger_storage.hpp> //for global logger default

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>

namespace ldmx {

    namespace logging {

        /**
         * Short names for boost namespaces
         */
        namespace log = boost::log;
        namespace sinks = boost::log::sinks;
        namespace level = log::trivial;

        /**
         * Define the type of logger we will be using in ldmx-sw
         */
        typedef log::sources::logger_mt logger;
    
        /**
         * Initialize the logging
         *
         * This function setups up the terminal and file sinks.
         * Sets their format and filtering level for this run.
         *
         * Boost Logging Trivial Levels:
         * logging::level::
         *      trace
         *      debug
         *      info
         *      warning
         *      error
         *      fatal
         *
         * @param termV logging level to output to terminal
         * @param fileV logging level to output to file
         * @param fileName name of log file if file logging is enabled
         * 
         * Defaults to batch setup where only fatal messages are allowed through the filters.
         */
        void open(int termV, int fileV, const std::string& fileName = "");

        /**
         * Close up the logging
         *
         * TODO need this before end of main()
         */
        void close();

    } //logging

} //ldmx

/**
 * Declare a global logger for all non-EventProcessors
 */
BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT( ldmx_log , ldmx::logging::logger )

#endif //EXCEPTION_LOGGER_H
