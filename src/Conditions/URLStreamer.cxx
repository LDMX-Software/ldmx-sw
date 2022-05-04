#include "Framework/Exception/Exception.h"
#include <fstream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include "Conditions/URLStreamer.h"
#include <string.h>
#include <unistd.h>
#include <iostream>

namespace conditions {

std::unique_ptr<std::istream> urlstream(const std::string& url) {
  if (url.find("file://")==0 || url.length()>0 && url[0]=='/') {
    std::string fname=url;
    if (fname.find("file://")==0) fname= url.substr(url.find("file://") + strlen("file://"));
    std::ifstream* fs=new std::ifstream(fname);
    if (!fs->good()) {
      delete fs;
      EXCEPTION_RAISE("ConditionsException",
                      "Unable to open CSV file '" + fname + "'");
    }
    return std::unique_ptr<std::istream>(fs);
  }
  if ((url.find("http://") != std::string::npos) ||
      (url.find("https://") != std::string::npos)) {

    // this implementation uses wget to handle the SSL processes
    static int istream = 0;
    char fname[250];
    snprintf(fname, 250, "/tmp/httpstream_%d_%d.csv ", getpid(), istream++);
    pid_t apid = fork();
    if (apid == 0) {  // child
      execl("/usr/bin/wget", "wget", "-q", "--no-check-certificate", "-O", fname, "-o", "/tmp/wget.log", url.c_str(), (char*)0);
    } else {
      int wstatus;
      int wrv = waitpid(apid, &wstatus, 0);
      //      std::cout << "EXITED: " << WIFEXITED(wstatus) << " STATUS: " << WEXITSTATUS(wstatus) << std::endl;
      if (WIFEXITED(wstatus)!=1 || WEXITSTATUS(wstatus)!=0) {
        EXCEPTION_RAISE("ConditionsException",
                        "Wget error "+std::to_string(WEXITSTATUS(wstatus))+ " retreiving URL '" + url + "'");
        
      }
    }
    std::ifstream ib(fname);
    if (ib.bad()) {
      EXCEPTION_RAISE("ConditionsException",
                        "Bad/empty file retreiving URL '" + url + "'");
    }
    std::stringstream* ss=new std::stringstream();
    (*ss) << ib.rdbuf();
    //    std::cout << "CONTENTS: \n" << ss->str();
    ib.close(); // needed for some implementations
    std::remove(fname);
    return std::unique_ptr<std::istream>(ss);
  }
  EXCEPTION_RAISE("ConditionsException",
                  "Unable to handle URL '" + url + "'");
  return std::unique_ptr<std::istream>(nullptr);
}

}  // namespace conditions
