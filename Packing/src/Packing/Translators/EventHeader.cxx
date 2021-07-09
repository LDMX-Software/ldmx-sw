
#include "Packing/Translators/EventHeader.h"

namespace packing {
namespace translators {

EventHeader::EventHeader(const framework::config::Parameters& ps) : Translator(ps) {
  i_run_ = ps.getParameter<int>("i_run");
  i_event_ = ps.getParameter<int>("i_event");
  i_time_ = ps.getParameter<int>("i_time");
  extra_param_names_ = ps.getParameter<std::string>("extra_param_names");
}

bool EventHeader::canTranslate(const std::string& name) const {
  return (name == "EventHeader");
}

void EventHeader::decode(framework::Event& event, const BufferType& buffer) {
  // Retrieve a mutable version of the event header
  ldmx::EventHeader& eh = event_->getEventHeader();

  // first buffer entry is run number
  eh.setRun(buffer.at(i_run));
  
  // second buffer entry is event number
  eh.setEventNumber(buffer.at(i_event));

  // third buffer entry is 64-bit time stamp for when data was taken since UTC epoch
  TTimeStamp stamp;
  stamp.Set(buffer.at(i_time),true,0,false);
  eh.setTimestamp(stamp);

  for (int i_param{0}; i_param < extra_param_names_.size(); i_param++) {
    eh.setIntParameter(extra_param_names_.at(i_param), buffer.at(i_param+3));
  }
}

}  // namespace translators
}  // namespace packing

DECLARE_PACKING_TRANSLATOR(packing::translators, EventHeader)
