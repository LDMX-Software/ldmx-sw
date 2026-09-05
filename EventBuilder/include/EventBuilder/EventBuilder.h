#ifndef EVENTBUILDER_EVENTBUILDER_H
#define EVENTBUILDER_EVENTBUILDER_H
#include <string>
#include <chrono>
#include "FragmentBuffer.h"
#include "Event/PhysicsEventData.h"
#include "Framework/EventProcessor.h"
#include "Framework/Logger.h"
#include "Packing/Utility/Reader.h"

namespace eventbuilder {

class EventBuilder : public framework::Producer {
 public:

  EventBuilder(const std::string &name, framework::Process &proc)
      : framework::Producer(name, proc), m_verbose_parse(false), m_event_id(0) {}
  
  enableLogging("EventBuilder")

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
  long long m_coherence_window_ns{5000000};  // 5 ms window for collecting fragments

  // Performance metrics
  std::chrono::steady_clock::time_point m_start_time;
  std::chrono::steady_clock::time_point m_event_start_time;
  uint64_t m_total_bytes_read{0};
  uint64_t m_total_events_built{0};
  unsigned long m_events_since_last_report{0};

  // Windowed metrics for real-time monitoring (DAQ use)
  static constexpr unsigned int WINDOW_SIZE = 100;  // Calculate rates over last 100 events
  std::chrono::steady_clock::time_point m_window_start_time;
  uint64_t m_window_events_count{0};
  uint64_t m_window_bytes_read{0};
};

}  // namespace eventbuilder
#endif