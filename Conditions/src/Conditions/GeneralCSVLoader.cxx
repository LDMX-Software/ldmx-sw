#include "Conditions/GeneralCSVLoader.h"

#include <string.h>
#include <wordexp.h>

#include <fstream>
#include <iostream>

#include "Framework/Exception/Exception.h"
#include "boost/tokenizer.hpp"

namespace conditions {

const std::string& GeneralCSVLoader::get(const std::string& colname,
                                         bool ignore_case) const {
  size_t i;
  for (i = 0; i < col_names_.size(); i++) {
    if (ignore_case && !strcasecmp(col_names_[i].c_str(), colname.c_str()))
      break;
    if (col_names_[i] == colname) break;
  }
  if (i == col_names_.size()) {
    EXCEPTION_RAISE("CSVNoSuchColumn",
                    "No such column '" + colname + "' reading CSV");
  }
  return row_data_[i];
}

int GeneralCSVLoader::getInteger(const std::string& colname,
                                 bool ignore_case) const {
  return atoi(get(colname, ignore_case).c_str());
}

bool GeneralCSVLoader::nextRow() {
  do {
    std::string line = getNextLine();
    if (line.empty()) return false;
    // it's a comment!
    if (line[0] == '#') continue;
    // split into pieces
    std::vector<std::string> line_split;
    // explicitly erase trailing white space
    line.erase(std::find_if(line.rbegin(), line.rend(),
                            [](unsigned char c) { return !std::isspace(c); })
                   .base(),
               line.end());
    boost::tokenizer<boost::escaped_list_separator<char>> tok(line);
    for (auto chunk : tok) {
      line_split.push_back(chunk);
    }
    if (col_names_.empty()) {
      col_names_.swap(line_split);
      continue;
    }
    if (line_split.size() == col_names_.size()) {
      row_data_.swap(line_split);
    } else {
      EXCEPTION_RAISE("CSVLineMismatch",
                      "Reading CSV found line with " +
                          std::to_string(line_split.size()) + " in CSV with " +
                          std::to_string(col_names_.size()) + " columns");
    }

    return true;
  } while (true);
}

StringCSVLoader::StringCSVLoader(const std::string& source,
                                 const std::string lineseparators)
    : source_{source}, linesep_{lineseparators}, row_begin_{0}, row_end_{0} {
  getNextLine();
}

std::string StringCSVLoader::getNextLine() {
  std::string retval;
  if (row_begin_ != row_end_) {
    if (row_end_ == std::string::npos) {
      retval = source_.substr(row_begin_, row_end_);
    } else {
      retval = source_.substr(row_begin_, row_end_ - row_begin_);
    }
  }
  // now we look for the follow on.
  // find the first non-end-of-line character
  row_begin_ = source_.find_first_not_of(linesep_, row_end_);
  if (row_begin_ != std::string::npos) {
    row_end_ = source_.find_first_of(linesep_, row_begin_);
  } else {
    row_end_ = std::string::npos;
  }
  return retval;
}

StreamCSVLoader::StreamCSVLoader(const std::string& filename)
    : source_{0}, own_stream_{true} {
  std::string expanded_fname = filename;
  wordexp_t p;

  if (wordexp(filename.c_str(), &p, 0)) {
    EXCEPTION_RAISE("StreamCSVFileNotFound",
                    "Error expanding '" + filename + "'");
  }

  if (!p.we_wordc) {
    EXCEPTION_RAISE("StreamCSVFileNotFound",
                    "No file matching '" + filename + "' found");
  }

  if (p.we_wordc != 1) {
    int nfound = p.we_wordc;
    wordfree(&p);
    EXCEPTION_RAISE("StreamCSVFileNotFound",
                    "Multiple files (" + std::to_string(nfound) +
                        ") matching '" + filename + "' found");
  }

  expanded_fname = p.we_wordv[0];
  wordfree(&p);

  source_ = new std::ifstream(expanded_fname);
  if (source_->fail()) {
    EXCEPTION_RAISE("StreamCSVFileNotFound",
                    "Unable to open '" + expanded_fname + "'");
  }
}

StreamCSVLoader::StreamCSVLoader(std::istream& stream)
    : source_{&stream}, own_stream_{false} {}

StreamCSVLoader::~StreamCSVLoader() {
  if (own_stream_ && source_) delete source_;
  source_ = 0;
}

std::string StreamCSVLoader::getNextLine() {
  // if no stream, no line...
  if (!source_) return "";

  do {
    line_.clear();
    std::getline(*source_, line_);
    if (line_.empty() && source_->eof()) return "";
  } while (line_.empty());  // skip blank lines

  return line_;
}

}  // namespace conditions
