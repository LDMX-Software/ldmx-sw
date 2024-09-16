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
  if (iLvl < 0)
    iLvl = 0;
  else if (iLvl > 4)
    iLvl = 4;
  return level(iLvl);
}

logger makeLogger(const std::string &name) {
  logger lg(log::keywords::channel = name);  // already has severity built in
  return boost::move(lg);
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
    const std::string& channel{*log::extract<std::string>(attrs["Channel"])};
    const level& msg_level{*log::extract<level>(attrs["Severity"])};
    auto it = custom_levels_.find(channel);
    if (it != custom_levels_.end()) {
      return msg_level >= it->second;
    }
    return msg_level >= fallback_level_;
  }
};


void open(const framework::config::Parameters& p) {
  // some helpful types
  typedef sinks::text_ostream_backend ourSinkBack_t;
  typedef sinks::synchronous_sink<ourSinkBack_t> ourSinkFront_t;

  level fileLevel{convertLevel(p.getParameter<int>("fileLevel", 0))};
  std::string filePath{p.getParameter<std::string>("filePath", "")};

  level termLevel{convertLevel(p.getParameter<int>("termLevel", 4))};
  const auto& logRules{p.getParameter<std::vector<framework::config::Parameters>>(
      "logRules", {})};
  std::unordered_map<std::string, level> custom_levels;
  for (const auto& logRule : logRules) {
    custom_levels[logRule.getParameter<std::string>("name")] = 
      convertLevel(logRule.getParameter<int>("level"));
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
  if (not filePath.empty()) {
    boost::shared_ptr<ourSinkBack_t> fileBack =
        boost::make_shared<ourSinkBack_t>();
    fileBack->add_stream(boost::make_shared<std::ofstream>(filePath));

    boost::shared_ptr<ourSinkFront_t> fileSink =
        boost::make_shared<ourSinkFront_t>(fileBack);

    // this is where the logging level is set
    fileSink->set_filter(Filter(fileLevel, custom_levels));
    fileSink->set_formatter([](const log::record_view &view, log::formatting_ostream &os) {
        Formatter::get()(view, os);
    });
    core->add_sink(fileSink);
  }  // file set to pass something

  // terminal sink is always created
  boost::shared_ptr<ourSinkBack_t> termBack =
      boost::make_shared<ourSinkBack_t>();
  termBack->add_stream(boost::shared_ptr<std::ostream>(
      &std::cout,            // point this stream to std::cout
      boost::null_deleter()  // don't let boost delete std::cout
      ));
  // flushes message to screen **after each message**
  termBack->auto_flush(true);

  boost::shared_ptr<ourSinkFront_t> termSink =
      boost::make_shared<ourSinkFront_t>(termBack);

  // translate integer level to enum
  termSink->set_filter(Filter(termLevel, custom_levels));
  // need to wrap formatter in lambda to enforce singleton formatter
  termSink->set_formatter([](const log::record_view &view, log::formatting_ostream &os) {
      Formatter::get()(view, os);
  });
  core->add_sink(termSink);

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

void Formatter::set(int n) {
  Formatter::get().event_number_ = n;
}

void Formatter::operator()(const log::record_view &view, log::formatting_ostream &os) {
  os << "[ " << log::extract<std::string>("Channel", view) << " ] "
     << event_number_ << " ";
  /**
   * We _copy_ the value out of the log into our own type
   * so that we can compare and convert it into a string.
   * I expect this copying to be okay since its just a
   * enum (int equivalent), but its good to be clear
   */
  level msg_level = *log::extract<level>("Severity", view);
  switch (msg_level) {
    case level::debug:
      os << "debug";
      break;
    case level::info:
      os << "info ";
      break;
    case level::warn:
      os << "warn ";
      break;
    case level::error:
      os << "error";
      break;
    case level::fatal:
      os << "fatal";
      break;
    default:
      os << "?????";
      break;
  }
  os << ": " << view[log::expressions::smessage];
}

}  // namespace logging

}  // namespace framework
