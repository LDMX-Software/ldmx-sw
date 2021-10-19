#ifndef HCAL_HCALRAWDECODER_H_ 
#define HCAL_HCALRAWDECODER_H_ 

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace hcal {

namespace utility {

/**
 * A reader that behaves the same for buffers coming from a raw file
 * or a buffer that is a vector of unsigned ints
 */
class Reader {
 public:
  Reader() : is_open_{false}, buffer_handle_{nullptr} {}

  void open(const std::vector<uint32_t> &b) {
    buffer_handle_ = &b;
    is_open_ = true;
  }
  void open(const std::string& file_name) {
    file_.unsetf(std::ios::skipws);
    file_.open(file_name, std::ios::binary|std::ios::in);
    is_open_ = true;
  }

  operator bool() const {
    if (!is_open_) 
      return false;
    else if (isFile()) {
      return bool(file_);
    } else 
      return (i_curr_ < buffer_handle_->size());
  }

  uint32_t next() {
    if (isFile())
      return file_pop();
    else
      return vector_pop();
  }

  Reader& operator>>(uint32_t& w) {
    w = next();
    return *this;
  }

  bool isFile() const {
    return (is_open_ and file_.is_open());
  }

  void rewind(unsigned int n) {
    if (isFile()) {
      std::cout << "Rewind: " << file_.tellg() << " -> ";
      file_.seekg(file_.tellg()-4*n);
      std::cout << file_.tellg() << std::endl;
    } else {
      i_curr_ - n;
    }
  }

 private:
  uint32_t vector_pop() {
    if (i_curr_+1 < buffer_handle_->size()) 
      ++i_curr_;
    return buffer_handle_->at(i_curr_);
  }

  uint32_t file_pop() {
    uint32_t w;
    if (file_)
      file_.read(reinterpret_cast<char*>(&w), 4);
    return w;
  }

 private:
  /// is the reader opened?
  bool is_open_;
  /// vector buffer reference
  const std::vector<uint32_t> *buffer_handle_{nullptr};
  /// current iterator
  long int i_curr_{-1};
  /// file stream reference
  std::ifstream file_;
};  // Reader

}  // namespace utility

/**
 * @class HcalRawDecoder
 */
class HcalRawDecoder : public framework::Producer {
 public:
  /**
   * Constructor
   */
  HcalRawDecoder(const std::string& name, framework::Process& process);

  /**
   * Destructor
   */
  virtual ~HcalRawDecoder();

  /**
   */
  virtual void configure(framework::config::Parameters&);

  /**
   */
  virtual void produce(framework::Event& event);

 private:
  /// input object of encoded data
  std::string input_name_;
  /// input pass of creating encoded data
  std::string input_pass_;
  /// output object to put onto event bus
  std::string output_name_;
  /// version of HGC ROC we are decoding
  int roc_version_;
  /// are get translating electronic IDs?
  bool translate_eid_;

 private:
  /// object to read raw data buffer
  utility::Reader reader_;

};
}  // namespace hcal

#endif
