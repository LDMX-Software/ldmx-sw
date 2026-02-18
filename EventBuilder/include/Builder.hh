#pragma once
#include <atomic>
#include <string>
#include <thread>
#include "FragmentBuffer.hh"
#include "PhysicsEventData.hh"
#include "Framework/EventProcessor.h"

namespace eventbuilder {

class Builder : public framework::Producer {
 public:
  /// Constructor required by the framework factory
  Builder(const std::string &name, framework::Process &proc)
      : framework::Producer(name, proc), m_app_running(true), m_verbose_parse(false), m_event_id(0) {}

  virtual ~Builder() = default;

  /// Configure the producer
  void configure(framework::config::Parameters &ps) override;

  /// Produce one event (called by the framework)
  void produce(framework::Event &event) override;

 private:
  bool parse_ror_header(const std::vector<uint32_t> &words, size_t index,
                        uint64_t &contrib, uint64_t &subsys, uint64_t &timestamp);
  bool try_parse_packing_subsystem(const std::vector<uint32_t> &words, size_t s,
                                   std::vector<uint32_t> &out_words, uint64_t &subsys,
                                   uint64_t &event_number_out);

  PhysicsEventData assemble_payload(const std::vector<DataFragment> &fragments);
  void write_event_binary(const PhysicsEventData &ev, const std::string &path = "events_out.dat");

  void eventMergerThread();

  std::atomic<bool> m_app_running;
  bool m_verbose_parse;
  unsigned int m_event_id;
  FragmentBuffer m_event_buffer;
  std::thread m_merger_thread;
  // Input file handling state
  std::string m_input_file;
  std::vector<uint32_t> m_words;
  std::vector<size_t> m_starts;
  size_t m_start_idx{0};
  bool m_input_loaded{false};
  std::string m_output_name{"BuilderOutput"};
};

}  // namespace eventbuilder
