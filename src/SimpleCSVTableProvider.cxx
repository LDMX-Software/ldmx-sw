#include "Conditions/SimpleCSVTableProvider.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
//#include <boost/asio/ssl.hpp>
#include <fstream>
#include "Conditions/SimpleTableStreamers.h"


DECLARE_CONDITIONS_PROVIDER_NS(ldmx,SimpleCSVTableProvider);

namespace ldmx {
    SimpleCSVTableProvider::~SimpleCSVTableProvider() { }
    
    SimpleCSVTableProvider::SimpleCSVTableProvider(const std::string& name, const std::string& tagname, const Parameters& parameters, Process& process) :
	ConditionsObjectProvider(name,tagname,parameters,process) {

	if (parameters.exists("provides")) { // Pythonic
	    std::vector<Parameters> plist=parameters.getParameter<std::vector<Parameters> > ("provides");
	    for (auto aprov: plist) {
		SimpleCSVTableProvider::Item item;
		item.objectName_=aprov.getParameter<std::string>("name");
		item.columns_=aprov.getParameter<std::vector<std::string>> ("columns");
		int firstRun=aprov.getParameter<int>("firstRun",-1);
		int lastRun=aprov.getParameter<int>("lastRun",-1);
		std::string rtype=aprov.getParameter<std::string>("runType","any");
		bool isMC=(rtype=="any" || rtype=="MC");
		bool isData=(rtype=="any" || rtype=="data");
		item.iov_=ConditionsIOV(firstRun,lastRun,isData,isMC);
		item.url_=aprov.getParameter<std::string>("URL");
		std::string dtype=aprov.getParameter<std::string>("dataType");
		if (dtype=="int" || dtype=="integer") item.objectType_=SimpleCSVTableProvider::Item::OBJ_int;
		if (dtype=="double" || dtype=="float") item.objectType_=SimpleCSVTableProvider::Item::OBJ_double;

		// here we check for overlaps
	
		for (auto tabledef : items_) {
		    if (tabledef.objectName_==item.objectName_ && item.iov_.overlaps(tabledef.iov_)) {
			std::stringstream err;
			err << "Table '" << tabledef.objectName_ << "' has entries with overlapping providers " << tabledef.url_ << " and " << item.url_;		   
			EXCEPTION_RAISE("ConditionsException",err.str());
		    }
		}
		// add to the list
		items_.push_back(item);
		if (std::find(objectNames_.begin(), objectNames_.end(), item.objectName_)==objectNames_.end()) objectNames_.push_back(item.objectName_);		
	    }
	}
	if (parameters.exists("CSVprovides")) { // CSVs
	}
    }

#define HACK_HACK
#ifndef HACK_HACK
    static void httpstream(const std::string& url, std::string& strbuf, int depth=0) {
	using boost::asio::ip::tcp;
	boost::asio::io_service io_service;

	int locslash=url.find("//");
	std::string hostname=url.substr(locslash+2,url.find("/",locslash+2)-locslash-2);
	std::string accessor=url.substr(0,locslash);

	std::cout << accessor << std::endl;
	
	// Get a list of endpoints corresponding to the server name.
	tcp::socket socket(io_service);
	try {
	    tcp::resolver resolver(io_service);
	    tcp::resolver::query query(hostname, accessor);
	    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	    
	    // Try each endpoint until we successfully establish a connection.
	    boost::asio::connect(socket, endpoint_iterator);
	} catch (...) {
	    EXCEPTION_RAISE("ConditionsException","Unable to access "+url+" to load conditions table.");
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
	    EXCEPTION_RAISE("ConditionsException","HTTP invalid response loading CSV file " + url + " from host " + hostname);
	}
    
	if (status_code >= 400) {
	    EXCEPTION_RAISE("ConditionsException","HTTP status code " + std::to_string(status_code) + " loading CSV file " + url + " from host " + hostname);
	}	

	if (status_code >= 300) { // redirect...
	    // Read the response headers, which are terminated by a blank line.
	    boost::asio::read_until(socket, response, "\r\n\r\n");

	    // Process the response headers.
	    std::string header;
	    std::string newurl;
	    while (std::getline(response_stream, header) && header != "\r")
		if (header.find("Location: ")!=std::string::npos)
		    newurl=header.substr(10);
	    if (newurl[newurl.size()-1]=='\r') newurl.erase(newurl.size()-1);		
		
	    //	    std::cout << "'" << newurl << "'\n";

	    //	    std::cout << newurl << "," << newhost <<"\n";      
					      
	    // read to the end for good behavior
	    boost::system::error_code error;
	    while (boost::asio::read(socket, response,
				     boost::asio::transfer_at_least(1), error));

	    if (depth>20) {
		EXCEPTION_RAISE("ConditionsException","Too much redirection in attempt to load HTTP CSV file");
	    }
            std::cout << newurl << " " << status_code << std::endl;
	    httpstream(newurl,strbuf,depth+1); // recursion...
	}

	// Read until EOF, writing data to output as we go.
	boost::system::error_code error;
	bool storing=false;
	while (boost::asio::read(socket, response,
				 boost::asio::transfer_at_least(1), error));

	while (response.size()>0) {
	    std::string s;
	    std::getline(response_stream, s);
	    if (s.empty() || s=="\r") {
		storing=true;
		continue;
	    }
	    if (storing) {
		strbuf+=s;
		strbuf+='\n';
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
static void httpstream(const std::string& url, std::string& strbuf, int depth=0) {
   static int istream=0;
   char fname[250];
   snprintf(fname,250,"/tmp/httpstream_%d_%d.csv ",getpid(),istream++);
   pid_t apid=fork();
   if (apid==0) { // child
     execl("/usr/bin/wget","wget","-q","-O",fname,url.c_str(),(char*)0);
   } else {
     int wstatus;
     int wrv=waitpid(apid,&wstatus,0);
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
	std::string::size_type j=0;
	for (std::string::size_type i=s.find("${",j); i<s.size(); i=s.find("${",j)) {
	    if (i!=j) retval.append(s,j,i-j); // append
	    j=s.find("}",i); // look for the end of request
	    std::string key=std::string(s,i+2,j-i-2);
	    if (key=="LDMX_CONDITION_TAG") retval+=getTagName();
	    else {
		const char* cenv=getenv(key.c_str());
		if (cenv!=0) {
		    retval+=cenv;
		}
		else {
		    // ERROR!
		}
	    }
	    j++;	    
	}
	if (j<s.size()) retval.append(s,j);
	//	std::cout << s << "=>" << retval << std::endl;
	return retval;
    }

    std::pair<const ConditionsObject*,ConditionsIOV> SimpleCSVTableProvider::getCondition(const std::string& condition_name, const EventHeader& context) {
	// put the condition tag into the environment for wordexp to use
	setenv("LDMX_CONDITION_TAG",getTagName().c_str(),1);

	for (auto tabledef : items_) {
	    //	    std::cout << condition_name << " " << tabledef.objectName_ << " " << tabledef.iov_ << " " << std::endl;
	    if (condition_name == tabledef.objectName_ && tabledef.iov_.validForEvent(context)) {
		std::string expurl=expandEnv(tabledef.url_);
		//		std::cout << "url:" <<  expurl << " from " << tabledef.url_ << std::endl;
		if (expurl.find("http://")!=std::string::npos || expurl.find("https://")!=std::string::npos) {
		    std::string buffer;
		    httpstream(expurl,buffer);
		    //std::cout <<buffer <<std::endl;
		    std::stringstream ss(buffer);
		    if (tabledef.objectType_==Item::OBJ_int) {
			IntegerTableCondition* table=new IntegerTableCondition(tabledef.objectName_,tabledef.columns_);
			ldmx::utility::SimpleTableStreamerCSV::load(*table,ss);
			return std::pair<const ConditionsObject*,ConditionsIOV>(table,tabledef.iov_);
		    } else if (tabledef.objectType_==Item::OBJ_double) {
			DoubleTableCondition* table=new DoubleTableCondition(tabledef.objectName_,tabledef.columns_);
			ldmx::utility::SimpleTableStreamerCSV::load(*table,ss);
			return std::pair<const ConditionsObject*,ConditionsIOV>(table,tabledef.iov_);
		    }
		} else if (expurl.find("file://")!=std::string::npos || expurl.find("://")==std::string::npos) {
		    std::string fname(expurl);
		    if (expurl.find("file://")!=std::string::npos) fname=expurl.substr(expurl.find("file://")+strlen("file://"));
		    std::ifstream fs(fname);
		    if (!fs.good()) {
			EXCEPTION_RAISE("ConditionsException","Unable to open CSV file '"+fname+"'");
		    }
		    if (tabledef.objectType_==Item::OBJ_int) {
			IntegerTableCondition* table=new IntegerTableCondition(tabledef.objectName_,tabledef.columns_);
			ldmx::utility::SimpleTableStreamerCSV::load(*table,fs);
			return std::pair<const ConditionsObject*,ConditionsIOV>(table,tabledef.iov_);
		    } else if (tabledef.objectType_==Item::OBJ_double) {
			DoubleTableCondition* table=new DoubleTableCondition(tabledef.objectName_,tabledef.columns_);
			ldmx::utility::SimpleTableStreamerCSV::load(*table,fs);
			return std::pair<const ConditionsObject*,ConditionsIOV>(table,tabledef.iov_);
		    }
		}			
	    }
	}
	return std::pair<const ConditionsObject*,ConditionsIOV>(0,ConditionsIOV());
    }
    
    void SimpleCSVTableProvider::runTest(const std::string& host, const std::string& path, std::string& ss) {

    }
}
