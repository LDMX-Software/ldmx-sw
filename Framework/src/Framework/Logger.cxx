#include "Framework/Logger.h"

// STL
#include <fstream>
#include <iostream>
#include <ostream>

// Boost
#include <boost/core/null_deleter.hpp>  //to avoid deleting std::cout
#include <boost/log/utility/setup/common_attributes.hpp>  //for loading commont attributes

namespace framework {

namespace logging {

/**
 * Convert an integer to the severity level enum
 *
 * Any integer below zero will be set to 0 (debug),
 * and any integer above four will be set to 4 (fatal).
 *
 * @param[in] iLvl integer level to be converted
 * @return converted enum level
 */
level convertLevel(int iLvl) {
  if (iLvl < -1)
    iLvl = -1;
  else if (iLvl > 4)
    iLvl = 4;
  return level(iLvl);
}

logger makeLogger(const std::string& name) {
  logger lg(log::keywords::channel = name);  // already has severity built in
  return boost::move(lg);
}

template <typename T>
const T& safeExtract(log::attribute_value attr) {
  static const T empty_value = {};
  auto attr_val = log::extract<T>(attr);
  if (attr_val) {
    return *attr_val;
  }
  return empty_value;
}

/**
 * Our filter implementation aligning with Boost.Log
 *
 * We store a default "fallback" level that is applied to all channels
 * that do not exist within the custom mapping. If a channel does exist
 * within the custom mapping, that value is used instead of the default
 * value.
 */
class Filter {
  level fallback_level_;
  std::unordered_map<std::string, level> custom_levels_;

 public:
  Filter(level fallback, std::unordered_map<std::string, level> custom)
      : fallback_level_{fallback}, custom_levels_{custom} {}
  Filter(level fallback) : Filter(fallback, {}) {}
  bool operator()(log::attribute_value_set const& attrs) {
    auto it = custom_levels_.find(safeExtract<std::string>(attrs["Channel"]));
    if (it != custom_levels_.end()) {
      return safeExtract<level>(attrs["Severity"]) >= it->second;
    }
    return safeExtract<level>(attrs["Severity"]) >= fallback_level_;
  }
};

void open(const framework::config::Parameters& p) {
  // some helpful types
  typedef sinks::text_ostream_backend ourSinkBack_t;
  typedef sinks::synchronous_sink<ourSinkBack_t> ourSinkFront_t;

  level file_level{convertLevel(p.getParameter<int>("fileLevel", 0))};
  std::string file_path{p.getParameter<std::string>("filePath", "")};

  level term_level{convertLevel(p.getParameter<int>("termLevel", 4))};
  std::vector<framework::config::Parameters> empty{};
  const auto& log_rules{
      p.get<std::vector<framework::config::Parameters>>("logRules", empty)};
  std::unordered_map<std::string, level> custom_levels;
  for (const auto& log_rule : log_rules) {
    custom_levels[log_rule.getParameter<std::string>("name")] =
        convertLevel(log_rule.getParameter<int>("level"));
  }

  // allow our logs to access common attributes, the ones availabe are
  //  "LineID"    : counter increments for each record being made (terminal or
  //  file) "TimeStamp" : time the log message was created "ProcessID" : machine
  //  ID for the process that is running "ThreadID"  : machine ID for the thread
  //  the message is in
  log::add_common_attributes();

  // get the core logging service
  boost::shared_ptr<log::core> core = log::core::get();

  // file sink is optional
  //  don't even make it if no filePath is provided
  if (not file_path.empty()) {
    boost::shared_ptr<ourSinkBack_t> file_back =
        boost::make_shared<ourSinkBack_t>();
    file_back->add_stream(boost::make_shared<std::ofstream>(file_path));

    boost::shared_ptr<ourSinkFront_t> file_sink =
        boost::make_shared<ourSinkFront_t>(file_back);

    // this is where the logging level is set
    file_sink->set_filter(Filter(file_level, custom_levels));
    file_sink->set_formatter(
        [](const log::record_view& view, log::formatting_ostream& os) {
          Formatter::get()(view, os);
        });
    core->add_sink(file_sink);
  }  // file set to pass something

  // terminal sink is always created
  boost::shared_ptr<ourSinkBack_t> term_back =
      boost::make_shared<ourSinkBack_t>();
  term_back->add_stream(boost::shared_ptr<std::ostream>(
      &std::cout,            // point this stream to std::cout
      boost::null_deleter()  // don't let boost delete std::cout
      ));
  // flushes message to screen **after each message**
  term_back->auto_flush(true);

  boost::shared_ptr<ourSinkFront_t> term_sink =
      boost::make_shared<ourSinkFront_t>(term_back);

  // translate integer level to enum
  term_sink->set_filter(Filter(term_level, custom_levels));
  // need to wrap formatter in lambda to enforce singleton formatter
  term_sink->set_formatter(
      [](const log::record_view& view, log::formatting_ostream& os) {
        Formatter::get()(view, os);
      });
  core->add_sink(term_sink);

  return;

}  // open

void close() {
  // prevents crashes on some systems when logging to a file
  log::core::get()->remove_all_sinks();

  return;
}

Formatter& Formatter::get() {
  static Formatter the_formatter;
  return the_formatter;
}

void Formatter::set(int n) { Formatter::get().event_number_ = n; }

void Formatter::operator()(const log::record_view& view,
                           log::formatting_ostream& os) {
  os << "[ " << log::extract<std::string>("Channel", view) << " ] "
     << event_number_ << " ";
  /**
   * We de-reference the value out of the log into our own type
   * so that we can compare and convert it into a string.
   */
  const level msg_level{safeExtract<level>(view["Severity"])};
  switch (msg_level) {
    case level::trace:
      os << "trace";
      break;
    case level::debug:
      os << "debug";
      break;
    case level::info:
      os << " info";
      break;
    case level::warn:
      os << " warn";
      break;
    case level::error:
      os << "error";
      break;
    case level::fatal:
      os << "fatal";
      break;
    default:
      // this should never happen since the loggers created
      // with makeLogger use the level enum and thus check
      // at compile time that the given level is an good option.
      // That being said, we leave this in case someone creates
      // their own logger circumventing makeLogger for whatever
      // reason and changes the enum defining the severity.
      os << "?????";
      break;
  }
  os << ": " << view[log::expressions::smessage];
}

}  // namespace logging

}  // namespace framework
