#pragma once
#include <string>
#include "FragmentBuffer.hh"
#include "Event/PhysicsEventData.hh"
#include "Framework/EventProcessor.h"
#include "Packing/Utility/Reader.h"

namespace eventbuilder {

class EventBuilder : public framework::Producer {
 public:

  EventBuilder(const std::string &name, framework::Process &proc)
      : framework::Producer(name, proc), m_verbose_parse(false), m_event_id(0) {}

  virtual ~EventBuilder() = default;

  void configure(framework::config::Parameters &ps) override;

  void produce(framework::Event &event) override;

 private:
 
  PhysicsEventData assemble_payload(const std::vector<DataFragment> &fragments);
  void write_event_binary(const PhysicsEventData &ev, const std::string &path = "events.bin");

  bool m_verbose_parse;
  unsigned int m_event_id;
  FragmentBuffer m_event_buffer;
  std::string m_input_file;
  packing::utility::Reader m_reader;
  std::string m_output_name{"BuilderOutput"};
};

}  // namespace eventbuilder
