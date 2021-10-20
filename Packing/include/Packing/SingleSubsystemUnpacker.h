#ifndef PACKING_SINGLESUBSYSTEMUNPACKER_H
#define PACKING_SINGLESUBSYSTEMUNPACKER_H

#include "Framework/EventProcessor.h"
#include "Framework/Exception/Exception.h"

#include "Packing/Utility/Reader.h"

namespace packing {

/**
 * @class SingleSubsystemUnpacker
 *
 * This producer unpacks the data from the a **single** subsystem
 * raw data file into a **single** buffer for a downstream processor
 * to decode.
 */
class SingleSubsystemUnpacker : public framework::Producer {
 public:
  /// normal constructor
  SingleSubsystemUnpacker(const std::string& name, framework::Process& p)
    : framework::Producer(name, p) {}
  /// empty destructor
  virtual ~SingleSubsystemUnpacker() {}

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
  void produce(framework::Event& event) final override;

 private:
  /**
   * Read the configured number of words of the input type
   * from the file and put that buffer onto the event bus
   * with the configured output_name_.
   */
  template <typename WordType>
  void read(framework::Event& event) {
    std::vector<WordType> buff;
    if(!reader_.read(buff, num_words_per_event_)) {
      EXCEPTION_RAISE("MalForm",
        "Raw file provided was unable to read "+std::to_string(num_words_per_event_)
        +" bytes in an event.");
    }
    event.add(output_name_, buff);
  }

 private:
  /// number of words per event to get from file
  int num_words_per_event_;
  /// number of bytes in each word
  int num_bytes_per_word_;
  /// destination object name
  std::string output_name_;

 private:
  /// raw data file we are reading
  utility::Reader reader_;
};  // SingleSubsystemUnpacker

}  // namespace packing

#endif  // PACKING_SINGLESUBSYSTEMUNPACKER_H
