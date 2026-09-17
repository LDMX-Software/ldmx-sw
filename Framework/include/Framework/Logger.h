#ifndef FRAMEWORK_LOGGER_H
#define FRAMEWORK_LOGGER_H

/**
 * Necessary to get linking to work?
 * https://stackoverflow.com/questions/23137637/linker-error-while-linking-boost-log-tutorial-undefined-references
 * https://www.boost.org/doc/libs/1_54_0/libs/log/doc/html/log/rationale/namespace_mangling.html
 * https://stackoverflow.com/a/40016057
 */
#define BOOST_ALL_DYN_LINK 1

// Keep this list minimal, every processor includes it (see issue #2107).
// Sinks, expressions, and setup headers belong in Logger.cxx.
#include <boost/log/core/record_view.hpp>
#include <boost/log/sources/record_ostream.hpp>  // BOOST_LOG_SEV
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/utility/formatting_ostream_fwd.hpp>
#include <string>

namespace framework {

namespace config {
class Parameters;
}  // namespace config

namespace logging {

/**
 * Severity/Logging levels
 */
enum level {
  trace = -1,
  debug = 0,  ///> 0
  info,       ///> 1
  warn,       ///> 2
  error,      ///> 3
  fatal       ///> 4
};

/**
 * Short name for boost namespace
 */
namespace log = boost::log;

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
 * @note Will not setup printing log messages to file if filePath is empty
 * string.
 *
 * @param p parameters to configure the logging with
 */
void open(const framework::config::Parameters& p);

/**
 * Close up the logging
 */
void close();

/**
 * Our logging formatter
 *
 * We use a singleton formatter so that it can hold the current event index_
 * as an attribute and include it within the logs. This is easier than
 * attempting to update the event number in all of the different logging
 * sources floating around ldmx-sw.
 */
class Formatter {
  int event_number_{0};
  Formatter() = default;

 public:
  /// delete the copy constructor
  Formatter(Formatter const&) = delete;

  /// delete the assignment operator
  void operator=(Formatter const&) = delete;

  /// get reference to the current single Formatter
  static Formatter& get();

  /// set the event number in the current Formatter
  static void set(int n);

  /**
   * format the passed record view into the output stream
   *
   * The format is
   *
   *  [ channel ] severity : message
   */
  void operator()(const log::record_view& view, log::formatting_ostream& os);
};

}  // namespace logging

}  // namespace framework

/**
 * @macro enableLogging
 * Enables logging in a class.
 *
 * Should be put in the 'private' section of the class
 * and before the closing bracket '};'
 *
 * Defines the member variable the_log_ with the input
 * name as the channel name.
 *
 * Makes the_log_ mutable so that the log can be used
 * in any class functions.
 */
#define enableLogging(name)                      \
  mutable ::framework::logging::logger the_log_{ \
      ::framework::logging::makeLogger(name)};

/**
 * @macro ldmx_log
 *
 * Assumes to have access to a variable named the_log_ of type logger.
 * Input logging level (without namespace or enum).
 */
// NOLINTNEXTLINE(readability-identifier-naming)
#define ldmx_log(lvl) BOOST_LOG_SEV(the_log_, ::framework::logging::level::lvl)

#endif  // FRAMEWORK_LOGGER_H
