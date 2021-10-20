#ifndef HCAL_HCALRAWDECODER_H_
#define HCAL_HCALRAWDECODER_H_

#include <fstream>

//----------//
//   LDMX   //
//----------//
#include "Framework/EventProcessor.h"

namespace hcal {

namespace utility {

/**
 * A reader that behaves the same for buffers coming from a raw file
 * or a buffer that is a vector of unsigned ints.
 *
 * The common aspect of both of these reading modes is that we
 * want to decode on a word-by-word basis where the words are
 * 32-bit integers.
 */
class Reader {
 public:
  /// construct a reader but it is not open yet
  Reader() : is_open_{false}, buffer_handle_{nullptr} {}

  /**
   * Open the reader with a vector to read
   */
  void open(const std::vector<uint32_t>& b);

  /**
   * Open the reader with a file to read.
   */
  void open(const std::string& file_name);

  /**
   * Check if the reader is in a good state.
   *
   * @return true if open and not at the end of file (or vector)
   */
  operator bool() const {
    if (!is_open_)
      return false;
    else if (isFile()) {
      return bool(file_);
    } else
      return (i_curr_ < buffer_handle_->size());
  }

  /**
   * Get the next word from the buffer
   *
   * @see file_pop for file mode
   * @see vector_pop for vector mode
   */
  uint32_t next();

  /**
   * Put the next word from the buffer in the passed reference.
   * Return the state of the reader after this is done.
   */
  Reader& operator>>(uint32_t& w) {
    w = next();
    return *this;
  }

  /**
   * Check if we are reading in file mode.
   *
   * @return true if we and the ifstream are both open
   */
  bool isFile() const { return (is_open_ and file_.is_open()); }

  /**
   * Sometimes we need to check the next few header words
   * but then re-read them later. This function allows us
   * to backtrack by n words.
   *
   * @param[in] n number of 32-bit words to back track by
   */
  void rewind(long int n);

 private:
  /**
   * Pop the next value of the buffer "off" the vector.
   * This just increments our index and returns the word
   * at that location.
   *
   * @throws std::out_of_range if we try to pop a word past
   * the end of the vector.
   */
  uint32_t vector_pop();

  /**
   * Pop the next word out of the file.
   * We only read from the file if the file is in a good
   * state so that we don't accidentally seg fault.
   * Otherwise we will just return a un-assigned word.
   */
  uint32_t file_pop();

 private:
  /// is the reader opened?
  bool is_open_;
  /// vector buffer reference
  const std::vector<uint32_t>* buffer_handle_{nullptr};
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
  HcalRawDecoder(const std::string& name, framework::Process& process)
      : framework::Producer(name, process) {}

  /**
   * Destructor
   */
  virtual ~HcalRawDecoder() = default;

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
  /// number of packets to read per event
  int num_packets_per_event_;
  /// are get translating electronic IDs?
  bool translate_eid_;

 private:
  /// object to read raw data buffer
  utility::Reader reader_;
};
}  // namespace hcal

#endif
