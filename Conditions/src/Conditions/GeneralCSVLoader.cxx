#include "Conditions/GeneralCSVLoader.h"
#include "Framework/Exception/Exception.h"
#include "boost/tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <string.h>
#include <wordexp.h>

namespace conditions {

const std::string& GeneralCSVLoader::get(const std::string& colname, bool ignore_case) const {
  size_t i;
  for (i=0; i<colNames_.size(); i++) {
    if (ignore_case && !strcasecmp(colNames_[i].c_str(),colname.c_str())) break;
    if (colNames_[i]==colname) break;
  }
  if (i==colNames_.size()) {
    EXCEPTION_RAISE("CSVNoSuchColumn","No such column '"+colname+"' reading CSV");
  }
  return rowData_[i];
}

int GeneralCSVLoader::getInteger(const std::string& colname, bool ignore_case) const {
  return atoi(get(colname, ignore_case).c_str());
}

bool GeneralCSVLoader::nextRow() {
  do {
    std::string line=getNextLine();
    if (line.empty()) return false;
    // it's a comment!
    if (line[0]=='#') continue;
    // split into pieces
    std::vector<std::string> line_split;
    // explicitly erase trailing white space
    line.erase(std::find_if(line.rbegin(), line.rend(),
          [](unsigned char c) { return !std::isspace(c); }).base(), line.end());
    boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
    for (auto chunk : tok){
      line_split.push_back(chunk);
    }
    if (colNames_.empty()) {
      colNames_.swap(line_split);
      continue;
    }
    if (line_split.size()==colNames_.size()) rowData_.swap(line_split);
    else {
      EXCEPTION_RAISE("CSVLineMismatch","Reading CSV found line with "+std::to_string(line_split.size())+" in CSV with "+std::to_string(colNames_.size()) +" columns");
    }

    return true;
  } while (true);
}

StringCSVLoader::StringCSVLoader(const std::string& source, const std::string lineseparators) : source_{source}, linesep_{lineseparators}, rowBegin_{0}, rowEnd_{0} {
  getNextLine();
}

std::string StringCSVLoader::getNextLine() {
  
  std::string retval;
  if (rowBegin_!=rowEnd_)
    if (rowEnd_==std::string::npos) retval=source_.substr(rowBegin_,rowEnd_);
    else retval=source_.substr(rowBegin_,rowEnd_-rowBegin_);

  // now we look for the follow on.
  // find the first non-end-of-line character
  rowBegin_=source_.find_first_not_of(linesep_,rowEnd_);
  if (rowBegin_!=std::string::npos) {
    rowEnd_=source_.find_first_of(linesep_,rowBegin_);
  } else rowEnd_=std::string::npos;

  return retval;
}

StreamCSVLoader::StreamCSVLoader(const std::string& filename) : source_{0},ownStream_{true} {
  std::string expanded_fname=filename;
  wordexp_t p;

  if (wordexp(filename.c_str(),&p,0)) {
    EXCEPTION_RAISE("StreamCSVFileNotFound","Error expanding '"+filename+"'");
  }

  if (!p.we_wordc) {
    EXCEPTION_RAISE("StreamCSVFileNotFound","No file matching '"+filename+"' found");
  }

  if (p.we_wordc!=1) {
    int nfound=p.we_wordc;
    wordfree(&p);
    EXCEPTION_RAISE("StreamCSVFileNotFound","Multiple files ("+std::to_string(nfound)+") matching '"+filename+"' found");
  }

  expanded_fname=p.we_wordv[0];
  wordfree(&p);
  
  source_=new std::ifstream(expanded_fname);
  if (source_->fail()) {
    EXCEPTION_RAISE("StreamCSVFileNotFound","Unable to open '"+expanded_fname+"'");
  }
}

StreamCSVLoader::StreamCSVLoader(std::istream& stream) : source_{&stream}, ownStream_{false} {
}

StreamCSVLoader::~StreamCSVLoader() {
  if (ownStream_ && source_) delete source_;
  source_=0;
}


std::string StreamCSVLoader::getNextLine() {
  // if no stream, no line...
  if (!source_) return "";

  do {
    line_.clear();
    std::getline(*source_,line_);
    if (line_.empty() && source_->eof()) return "";
  } while (line_.empty()); // skip blank lines

  return line_;
}

}
