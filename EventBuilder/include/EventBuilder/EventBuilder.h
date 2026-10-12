#ifndef EVENTBUILDER_EVENTBUILDER_H
#define EVENTBUILDER_EVENTBUILDER_H
#include <chrono>
#include <string>

#include "Event/PhysicsEventData.h"
#include "FragmentBuffer.h"
#include "Framework/EventProcessor.h"
#include "Framework/Logger.h"
#include "Packing/Utility/Reader.h"

namespace eventbuilder {

class EventBuilder : public framework::Producer {
 public:
  EventBuilder(const std::string& name, framework::Process& proc)
      : framework::Producer(name, proc),
        m_verbose_parse_(false),
        m_event_id_(0) {}

  // trailing ';' keeps clang-format from indenting what follows
  enableLogging("EventBuilder");

  virtual ~EventBuilder() = default;

  void configure(framework::config::Parameters& ps) override;

  void produce(framework::Event& event) override;

 private:
  PhysicsEventData assemblePayload(const std::vector<DataFragment>& fragments);
  void writeEventBinary(const PhysicsEventData& ev,
                        const std::string& path = "events.bin");

  bool m_verbose_parse_;
  unsigned int m_event_id_;
  FragmentBuffer m_event_buffer_;
  std::string m_input_file_;
  packing::utility::Reader m_reader_;
  std::string m_output_name_{"BuilderOutput"};
  long long m_coherence_window_ns_{
      5000000};  // 5 ms window for collecting fragments
  // Minimum number of distinct subsystems required to assemble an event.
  // Configurable (was a hardcoded 3); Run 182 only has ts + tracker, so 2.
  int m_min_subsystems_{2};

  // Performance metrics
  std::chrono::steady_clock::time_point m_start_time_;
  std::chrono::steady_clock::time_point m_event_start_time_;
  uint64_t m_total_bytes_read_{0};
  uint64_t m_total_events_built_{0};
  unsigned long m_events_since_last_report_{0};

  // Windowed metrics for real-time monitoring (DAQ use)
  static constexpr unsigned int WINDOW_SIZE =
      100;  // Calculate rates over last 100 events
  std::chrono::steady_clock::time_point m_window_start_time_;
  uint64_t m_window_events_count_{0};
  uint64_t m_window_bytes_read_{0};
};

}  // namespace eventbuilder
#endif