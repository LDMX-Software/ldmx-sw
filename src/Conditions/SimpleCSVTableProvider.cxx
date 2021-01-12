#include "Conditions/SimpleCSVTableProvider.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
//#include <boost/asio/ssl.hpp>
#include <fstream>
#include "Conditions/SimpleTableStreamers.h"

DECLARE_CONDITIONS_PROVIDER_NS(conditions, SimpleCSVTableProvider);

namespace conditions {

SimpleCSVTableProvider::~SimpleCSVTableProvider() {}

SimpleCSVTableProvider::SimpleCSVTableProvider(const std::string& name,
                                               const std::string& tagname,
                                               const framework::config::Parameters& parameters,
                                               framework::Process& process)
    : framework::ConditionsObjectProvider(name, tagname, parameters, process) {
  columns_ = parameters.getParameter<std::vector<std::string>>("columns");
  std::string dtype = parameters.getParameter<std::string>("dataType");
  if (dtype == "int" || dtype == "integer")
    objectType_ = SimpleCSVTableProvider::OBJ_int;
  if (dtype == "double" || dtype == "float")
    objectType_ = SimpleCSVTableProvider::OBJ_double;

  std::vector<framework::config::Parameters> plist =
      parameters.getParameter<std::vector<framework::config::Parameters>>("entries");
  for (auto aprov : plist) {
    SimpleCSVTableProvider::Entry item;
    int firstRun = aprov.getParameter<int>("firstRun", -1);
    int lastRun = aprov.getParameter<int>("lastRun", -1);
    std::string rtype = aprov.getParameter<std::string>("runType", "any");
    bool isMC = (rtype == "any" || rtype == "MC");
    bool isData = (rtype == "any" || rtype == "data");
    item.iov_ = framework::ConditionsIOV(firstRun, lastRun, isData, isMC);
    item.url_ = aprov.getParameter<std::string>("URL");
    if (objectType_ == OBJ_int && aprov.exists("values")) {
      item.ivalues_ = aprov.getParameter<std::vector<int>>("values");
      if (item.ivalues_.size() != columns_.size()) {
        EXCEPTION_RAISE("ConditionsException",
                        "Mismatch in values vector (" +
                            std::to_string(item.ivalues_.size()) +
                            ") and columns vector (" +
                            std::to_string(columns_.size()) + ") in " +
                            getConditionObjectName());
      }
    }
    if (objectType_ == OBJ_double && aprov.exists("values")) {
      item.dvalues_ = aprov.getParameter<std::vector<double>>("values");
      if (item.dvalues_.size() != columns_.size()) {
        EXCEPTION_RAISE("ConditionsException",
                        "Mismatch in values vector (" +
                            std::to_string(item.dvalues_.size()) +
                            ") and columns vector (" +
                            std::to_string(columns_.size()) + ") in " +
                            getConditionObjectName());
      }
    }
    // here we check for overlaps

    for (auto tabledef : entries_) {
      if (item.iov_.overlaps(tabledef.iov_)) {
        std::stringstream err;
        err << "Table '" << getConditionObjectName()
            << "' has entries with overlapping providers " << tabledef.url_
            << " and " << item.url_;
        EXCEPTION_RAISE("ConditionsException", err.str());
      }
    }
    // add to the list
    entries_.push_back(item);
  }
}

#define HACK_HACK
#ifndef HACK_HACK
static void httpstream(const std::string& url, std::string& strbuf,
                       int depth = 0) {
  using boost::asio::ip::tcp;
  boost::asio::io_service io_service;

  int locslash = url.find("//");
  std::string hostname =
      url.substr(locslash + 2, url.find("/", locslash + 2) - locslash - 2);

  // Get a list of endpoints corresponding to the server name.
  tcp::socket socket(io_service);
  try {
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(hostname, "http");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    // Try each endpoint until we successfully establish a connection.
    boost::asio::connect(socket, endpoint_iterator);
  } catch (...) {
    EXCEPTION_RAISE("ConditionsException",
                    "Unable to access " + url + " to load conditions table.");
  }

  // Form the request. We specify the "Connection: close" header so that the
  // server will close the socket after transmitting the response. This will
  // allow us to treat all data up until the EOF as the content.
  boost::asio::streambuf request;
  std::ostream request_stream(&request);
  request_stream << "GET " << url << " HTTP/1.0\r\n";
  request_stream << "Host: " << hostname << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";

  // Send the request.
  boost::asio::write(socket, request);

  // Read the response status line. The response streambuf will automatically
  // grow to accommodate the entire line. The growth may be limited by passing
  // a maximum size to the streambuf constructor.
  boost::asio::streambuf response;
  boost::asio::read_until(socket, response, "\r\n");

  // Check that response is OK.
  std::istream response_stream(&response);
  std::string http_version;
  response_stream >> http_version;
  unsigned int status_code;
  response_stream >> status_code;
  std::string status_message;
  std::getline(response_stream, status_message);
  if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
    EXCEPTION_RAISE("ConditionsException",
                    "HTTP invalid response loading CSV file " + url +
                        " from host " + hostname);
  }

  if (status_code >= 400) {
    EXCEPTION_RAISE("ConditionsException",
                    "HTTP status code " + std::to_string(status_code) +
                        " loading CSV file " + url + " from host " + hostname);
  }

  if (status_code >= 300) {  // redirect...
    // Read the response headers, which are terminated by a blank line.
    boost::asio::read_until(socket, response, "\r\n\r\n");

    // Process the response headers.
    std::string header;
    std::string newurl;
    while (std::getline(response_stream, header) && header != "\r")
      if (header.find("Location: ") != std::string::npos)
        newurl = header.substr(10);
    if (newurl[newurl.size() - 1] == '\r') newurl.erase(newurl.size() - 1);

    //	    std::cout << "'" << newurl << "'\n";

    //	    std::cout << newurl << "," << newhost <<"\n";

    // read to the end for good behavior
    boost::system::error_code error;
    bool storing = false;
    while (boost::asio::read(socket, response,
                             boost::asio::transfer_at_least(1), error))
      ;

    if (depth > 10) {
      EXCEPTION_RAISE("ConditionsException",
                      "Too much redirection in attempt to load HTTP CSV file");
    }
    httpstream(newurl, strbuf, depth + 1);  // recursion...
  }

  // Read until EOF, writing data to output as we go.
  boost::system::error_code error;
  bool storing = false;
  while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1),
                           error))
    ;

  while (response.size() > 0) {
    std::string s;
    std::getline(response_stream, s);
    if (s.empty() || s == "\r") {
      storing = true;
      continue;
    }
    if (storing) {
      strbuf += s;
      strbuf += '\n';
    }
  }

  //	if (error != boost::asio::error::eof)
  //   throw boost::system::system_error(error);
  //	strbuf=build.str();
}
#endif
#ifdef HACK_HACK
#include <sys/wait.h>
// horrible hack using wget
static void httpstream(const std::string& url, std::string& strbuf,
                       int depth = 0) {
  static int istream = 0;
  char fname[250];
  snprintf(fname, 250, "/tmp/httpstream_%d_%d.csv ", getpid(), istream++);
  pid_t apid = fork();
  if (apid == 0) {  // child
    execl("/usr/bin/wget", "wget", "-q", "-O", fname, url.c_str(), (char*)0);
  } else {
    int wstatus;
    int wrv = waitpid(apid, &wstatus, 0);
  }

  std::ifstream ib(fname);
  std::ostringstream ss;
  ss << ib.rdbuf();
  strbuf = ss.str();
  std::remove(fname);
}

#endif

std::string SimpleCSVTableProvider::expandEnv(const std::string& s) const {
  std::string retval;
  std::string::size_type j = 0;
  for (std::string::size_type i = s.find("${", j); i < s.size();
       i = s.find("${", j)) {
    if (i != j) retval.append(s, j, i - j);  // append
    j = s.find("}", i);                      // look for the end of request
    std::string key = std::string(s, i + 2, j - i - 2);
    if (key == "LDMX_CONDITION_TAG")
      retval += getTagName();
    else {
      const char* cenv = getenv(key.c_str());
      if (cenv != 0) {
        retval += cenv;
      } else {
        // ERROR!
      }
    }
    j++;
  }
  if (j < s.size()) retval.append(s, j);
  //	std::cout << s << "=>" << retval << std::endl;
  return retval;
}

std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>
SimpleCSVTableProvider::getCondition(const framework::EventHeader& context) {
  // put the condition tag into the environment for wordexp to use
  setenv("LDMX_CONDITION_TAG", getTagName().c_str(), 1);

  for (auto tabledef : entries_) {
    //	    std::cout << condition_name << " " << tabledef.objectName_ << " " <<
    //tabledef.iov_ << " " << std::endl;
    if (tabledef.iov_.validForEvent(context)) {
      std::string expurl = expandEnv(tabledef.url_);
      //		std::cout << "url:" <<  expurl << " from " <<
      //tabledef.url_ << std::endl;
      if (expurl.find("http://") != std::string::npos) {
        std::string buffer;
        httpstream(expurl, buffer);
        // std::cout <<buffer <<std::endl;
        std::stringstream ss(buffer);
        if (objectType_ == OBJ_int) {
          IntegerTableCondition* table =
              new IntegerTableCondition(getConditionObjectName(), columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, ss);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        } else if (objectType_ == OBJ_double) {
          conditions::DoubleTableCondition* table =
              new conditions::DoubleTableCondition(getConditionObjectName(), columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, ss);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        }
      } else if (expurl.find("file://") != std::string::npos ||
                 expurl.find(":") == std::string::npos) {
        std::string fname(expurl);
        if (expurl.find("file://") != std::string::npos)
          fname = expurl.substr(expurl.find("file://") + strlen("file://"));
        std::ifstream fs(fname);
        if (!fs.good()) {
          EXCEPTION_RAISE("ConditionsException",
                          "Unable to open CSV file '" + fname + "'");
        }
        if (objectType_ == OBJ_int) {
          IntegerTableCondition* table =
              new IntegerTableCondition(getConditionObjectName(), columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, fs);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        } else if (objectType_ == OBJ_double) {
          conditions::DoubleTableCondition* table =
              new conditions::DoubleTableCondition(getConditionObjectName(), columns_);
          conditions::utility::SimpleTableStreamerCSV::load(*table, fs);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        }
      } else if (expurl == "python:") {
        // here we just copy values...
        if (objectType_ == OBJ_int) {
          IntegerTableCondition* table =
              new IntegerTableCondition(getConditionObjectName(), columns_);
          table->setIdMask(0);  // all ids are the same...
          table->add(0, tabledef.ivalues_);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        } else if (objectType_ == OBJ_double) {
          conditions::DoubleTableCondition* table =
              new conditions::DoubleTableCondition(getConditionObjectName(), columns_);
          table->setIdMask(0);  // all ids are the same...
          table->add(0, tabledef.dvalues_);
          return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(
              table, tabledef.iov_);
        }
      }
    }
  }
  return std::pair<const framework::ConditionsObject*, framework::ConditionsIOV>(0, framework::ConditionsIOV());
}

}  // namespace conditions
