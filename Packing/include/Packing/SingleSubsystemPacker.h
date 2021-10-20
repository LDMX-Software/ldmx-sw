#ifndef PACKING_SINGLESUBSYSTEMPACKER_H
#define PACKING_SINGLESUBSYSTEMPACKER_H

#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"

#include "Packing/Utility/Writer.h"

namespace packing {

/**
 * @class SingleSubsystemPacker
 *
 * This producer unpacks the data from the a **single** subsystem
 * raw data file into a **single** buffer for a downstream processor
 * to decode.
 */
class SingleSubsystemPacker : public framework::Analyzer {
 public:
  /// normal constructor
  SingleSubsystemPacker(const std::string& name, framework::Process& p)
    : framework::Analyzer(name, p) {}
  /// empty destructor
  virtual ~SingleSubsystemPacker() {}

  /**
   * Configure the unpacker and open the raw data file for IO
   *
   * @param[in] ps Parameters for configuration
   */
  void configure(framework::config::Parameters& ps) final override;

  /**
   * Actually do the unpacking/decoding.
   *
   * @param[in,out] event Event bus with raw data where we will put the digis
   */
  void analyze(const framework::Event& event) final override;

 private:
  /**
   * Write the buffer retrieved from the event bus
   * using the input type as the word type and the
   * configured object/pass names.
   */
  template <typename WordType>
  void write(const framework::Event& event) {
    auto buff{event.getCollection<WordType>(input_name_,input_pass_)};
    writer_ << buff;
  }

 private:
  /// buffer object name on event bus
  std::string input_name_;
  /// buffer object pass on event bus
  std::string input_pass_;
  /// number of bytes in a single word
  int num_bytes_per_word_;

 private:
  /// raw data file we are reading
  utility::Writer writer_;
};  // SingleSubsystemPacker

}  // namespace packing

#endif  // PACKING_SINGLESUBSYSTEMPACKER_H
