#ifndef PACKING_TRANSLATORS_EVENTHEADER_H
#define PACKING_TRANSLATORS_EVENTHEADER_H

#include "Framework/EventHeader.h"

#include "Packing/Translator.h"

namespace packing {
namespace translators {

/**
 * @class EventHeader
 * Translator for data coming out of the HGC ROC.
 */
class EventHeader : public Translator {
 public:
  /**
   * Constructor
   *
   * There are three required parameters.
   *  i_run : Index in buffer of run number (between 0 and 2)
   *  i_event : Index in buffer of event number (between 0 and 2)
   *  i_time : index in buffer of time stamp (between 0 and 2)
   * The rest of the buffer can be used to copy other integer
   * parameters into the event header. The name of those parameters
   * is provided by extra_param_names.
   */
  EventHeader(const framework::config::Parameters& ps);

  /// default destructor
  virtual ~EventHeader() = default;

  /**
   * We can translate the EventHeader
   * @param[in] name Name of data stream to check
   * @return true if name is 'EventHeader'
   */
  bool canTranslate(const std::string& name) const final override;
  
  /**
   * Decode the input buffer into an event header
   *
   * @param[in,out] event EventBus to put object onto
   * @param[in] buffer RawData buffer to decode
   */
  void decode(framework::Event& event, const BufferType& buffer) final override;
  
  /**
   * Encoding the C++ digi into a raw event type is delayed
   * until we more strictly define our resulting raw data format.
   */
  BufferType encode(const ldmx::EventHeader& data) {
    EXCEPTION_RAISE("NoImp",
        "EventHeader Translator hasn't implemented encode yet.");
    return {};
  }

 private:
  /// Index of run number
  int i_run_;
  /// Index of event number
  int i_event_;
  /// Index of time stamp
  int i_time_;
  /// Names for any other parameters in buffer
  std::vector<std::string> extra_param_names_;
};

}  // namespace translators
}  // namespace packing

#endif  // PACKING_TRANSLATORS_EVENTHEADER_H
