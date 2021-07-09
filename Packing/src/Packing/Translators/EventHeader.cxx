
#include "Packing/Translators/EventHeader.h"

namespace packing {
namespace translators {

EventHeader::EventHeader(const framework::config::Parameters& ps) : Translator(ps) {
  i_run_ = ps.getParameter<int>("i_run");
  i_event_ = ps.getParameter<int>("i_event");
  i_time_ = ps.getParameter<int>("i_time");
  extra_param_names_ = ps.getParameter<std::vector<std::string>>("extra_param_names");
}

bool EventHeader::canTranslate(const std::string& name) const {
  return (name == "EventHeader");
}

void EventHeader::decode(framework::Event& event, const BufferType& buffer) {
  // Retrieve a mutable version of the event header
  ldmx::EventHeader& eh = event.getEventHeader();

  // copy over run and event numbers
  eh.setRun(buffer.at(i_run_));
  eh.setEventNumber(buffer.at(i_event_));

  // convert time stamp to ROOT's object
  eh.setTimestamp(TTimeStamp(buffer.at(i_time_),true,0,false));

  // copy over extra parameters in buffer,
  //  we assume that they come after the three required parameters
  for (int i_param{0}; i_param < extra_param_names_.size(); i_param++) {
    eh.setIntParameter(extra_param_names_.at(i_param), buffer.at(i_param+3));
  }
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, EventHeader)
