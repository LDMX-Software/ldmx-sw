#ifndef FRAMEWORK_LOGGER_H
#define FRAMEWORK_LOGGER_H

/**
 * Necessary to get linking to work?
 * https://stackoverflow.com/questions/23137637/linker-error-while-linking-boost-log-tutorial-undefined-references
 * https://www.boost.org/doc/libs/1_54_0/libs/log/doc/html/log/rationale/namespace_mangling.html
 * https://stackoverflow.com/a/40016057
 */
#define BOOST_ALL_DYN_LINK 1

#include <boost/log/core.hpp>                 //core logging service
#include <boost/log/expressions.hpp>          //for attributes and expressions
#include <boost/log/sinks/sync_frontend.hpp>  //syncronous sink frontend
#include <boost/log/sinks/text_ostream_backend.hpp>  //output stream sink backend
#include <boost/log/sources/global_logger_storage.hpp>  //for global logger default
#include <boost/log/sources/severity_channel_logger.hpp>  //for the severity logger
#include <boost/log/sources/severity_feature.hpp>  //for the severity feature in a logger
#include <boost/log/utility/setup/common_attributes.hpp>

// TODO check which headers are required
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace framework {

namespace logging {

/**
 * Severity/Logging levels
 */
enum level {
  debug = 0,  ///> 0
  info,       ///> 1
  warn,       ///> 2
  error,      ///> 3
  fatal       ///> 4
};

/**
 * Convert an integer to the severity level enum
 *
 * Any integer below zero will be set to 0 (debug),
 * and any integer above four will be set to 4 (fatal).
 *
 * @param[in] iLvl integer level to be converted
 * @return converted enum level
 */
level convertLevel(int& iLvl);

/**
 * Severity level to human readable name map
 *
 * Human readable names are the same width in characters
 *
 * We could also add terminal color here if we wanted.
 *
 * @note Not in use right now. boost's extract returns some
 * weird templated type that cannot be implicitly converted to level.
 *
 * We _can_ stream the output to a string stream and use
 * that as the key in our map, but that is lame.
 */
const std::unordered_map<level, std::string> humanReadableLevel = {
    {debug, "debug"},
    {info, "info "},
    {warn, "warn "},
    {error, "error"},
    {fatal, "fatal"}};

/**
 * Short names for boost namespaces
 */
namespace log = boost::log;
namespace sinks = boost::log::sinks;

/**
 * Define the type of logger we will be using in ldmx-sw
 */
typedef log::sources::severity_channel_logger_mt<level, std::string> logger;

/**
 * Gets a logger for the user
 *
 * Returns a logger type with some extra initialization procedures done.
 * Should _only be called ONCE_ during a run.
 *
 * @note Use the enableLogging macro in your class declaration instead
 * of this function directly.
 *
 * @param name name of this logging channel (e.g. processor name)
 * @return logger with the input channel name
 */
logger makeLogger(const std::string& name);

/**
 * Initialize the logging backend
 *
 * This function setups up the terminal and file sinks.
 * Sets their format and filtering level for this run.
 *
 * @note Will not setup printing log messages to file if fileName is empty
 * string.
 *
 * @param termLevel minimum level to print to terminal (everything above it is
 * also printed)
 * @param fileLevel minimum level to print to file log (everything above it is
 * also printed)
 * @param fileName name of file to print log to
 */
void open(const level termLevel, const level fileLevel,
          const std::string& fileName);

/**
 * Close up the logging
 */
void close();

}  // namespace logging

}  // namespace framework

/**
 * @macro enableLogging
 * Enables logging in a class.
 *
 * Should be put in the 'private' section of the class
 * and before the closing bracket '};'
 *
 * Defines the member variable theLog_ with the input
 * name as the channel name.
 *
 * Makes theLog_ mutable so that the log can be used
 * in any class functions.
 */
#define enableLogging(name)                     \
  mutable ::framework::logging::logger theLog_{ \
      ::framework::logging::makeLogger(name)};

/**
 * @macro ldmx_log
 *
 * Assumes to have access to a variable named theLog_ of type logger.
 * Input logging level (without namespace or enum).
 */
#define ldmx_log(lvl) BOOST_LOG_SEV(theLog_, ::framework::logging::level::lvl)

#endif  // FRAMEWORK_LOGGER_H
