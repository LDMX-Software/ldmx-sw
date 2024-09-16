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

level convertLevel(int &iLvl) {
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

void open(const level termLevel, const level fileLevel,
          const std::string &fileName) {
  // some helpful types
  typedef sinks::text_ostream_backend ourSinkBack_t;
  typedef sinks::synchronous_sink<ourSinkBack_t> ourSinkFront_t;

  // allow our logs to access common attributes, the ones availabe are
  //  "LineID"    : counter increments for each record being made (terminal or
  //  file) "TimeStamp" : time the log message was created "ProcessID" : machine
  //  ID for the process that is running "ThreadID"  : machine ID for the thread
  //  the message is in
  log::add_common_attributes();

  // get the core logging service
  boost::shared_ptr<log::core> core = log::core::get();

  // file sink is optional
  //  don't even make it if no fileName is provided
  if (not fileName.empty()) {
    boost::shared_ptr<ourSinkBack_t> fileBack =
        boost::make_shared<ourSinkBack_t>();
    fileBack->add_stream(boost::make_shared<std::ofstream>(fileName));

    boost::shared_ptr<ourSinkFront_t> fileSink =
        boost::make_shared<ourSinkFront_t>(fileBack);

    // this is where the logging level is set
    fileSink->set_filter(log::expressions::attr<level>("Severity") >=
                         fileLevel);
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
  termSink->set_filter(log::expressions::attr<level>("Severity") >= termLevel);
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
  os << " [ " << log::extract<std::string>("Channel", view) << " ] "
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
