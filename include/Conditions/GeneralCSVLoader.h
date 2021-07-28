/**
 * @file The GeneralCSVLoader parses a CSV file and provides the rows one at a time to a user
 */
#ifndef CONDITIONS_GENERALCSVLOADER_H_
#define CONDITIONS_GENERALCSVLOADER_H_

#include <string>
#include <vector>
#include <istream>

namespace conditions {

/** @brief Class which parses a CSV file and provides the rows one at a time to a user
 * The parser ignores any line which begins with a '#' character
 * The parser uses the first non-comment row to determine column names
 * The parser handles quotation marks in a standard manner
 */
class GeneralCSVLoader {
 public:
  /** Destructor */
  virtual ~GeneralCSVLoader()  { }
  
  /** Get the column names */
  std::vector<std::string> columnNames() const { return colNames_; }

  /** Advance to next row if possible */
  bool nextRow();

  /** Get the value for the given column in the current row */
  const std::string& get(const std::string& colname, bool ignore_case=true) const;

  /** Get the value for the given column in the current row as an integer*/
  int getInteger(const std::string& colname, bool ignore_case=true) const;

 protected:

  GeneralCSVLoader() { }

  /** Get the next line, returning an empty string when there is no further data */
  virtual std::string getNextLine() = 0;

 private:
  /**
   * The column names
   */
  std::vector<std::string> colNames_;
  /**
   * The row data
   */
  std::vector<std::string> rowData_;
};

/** @brief Specialization of the GeneralCSVLoader for loading from a string
 */
class StringCSVLoader : public GeneralCSVLoader {
 public:
  /** Constructor */
  StringCSVLoader(const std::string& source, const std::string lineseparators="\n");

 protected:
  /** Get the next line, returning an empty string when there is no further data */
  virtual std::string getNextLine();

 private:
  /**
   * The original string
   */
  const std::string& source_;
  /** 
   * The separators
   */
  const std::string linesep_;
  /**
   * The current start and end pointers
   */
  std::string::size_type rowBegin_, rowEnd_;
};

/** @brief Specialization of the GeneralCSVLoader for loading from a file/stream
 */
class StreamCSVLoader : public GeneralCSVLoader {
 public:
  /** 
   * Constructor a loader from the provided file name
   *
   * We expand the file-name using wordexp and then open an
   * input file stream to it. We own the stream in this case.
   */
  StreamCSVLoader(const std::string& filename);
  /** 
   * Construct a loader from the provided input stream
   *
   * We are given an already-created stream, so
   * we do not own the stream in this case.
   */
  StreamCSVLoader(std::istream& stream);

  /// Clean-up the stream if we own it
  virtual ~StreamCSVLoader();
  
 protected:
  /** Get the next line, returning an empty string when there is no further data */
  virtual std::string getNextLine();

 private:
  /**
   * The stream
   */
  std::istream* source_;
  /** 
   * Own stream?
   */
  bool ownStream_;
  /** 
   * Line buffer
   */
  std::string line_;
};

}
#endif // CONDITIONS_GENERALCSVLOADER_H_
