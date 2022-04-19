
#include <queue>

#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace hcal {

/**
 * Align the two polarfires with drop/keep hints signalling successful merge
 *
 * Only checking for /dropped/ events, assuming that ticks and spills are already
 * in correct order.
 */
class HcalAlignPolarfires : public framework::Producer {
  /// input decoded objects (vector index == polarfire index)
  std::vector<std::string> input_names_;
  /// pass name for decoded objects
  std::string input_pass_;
  /// output object name
  std::string output_name_;
  /// number of 5MHz ticks difference to consider polarfires aligned
  static int max_tick_diff_;
 public:
  struct PolarfireQueueEntry {
    int spill;
    int ticks;
    ldmx::HgcrocDigiCollection digis;
    PolarfireQueueEntry(const framework::Event& event, 
        const std::string& input_name, const std::string& input_pass) {
      spill = event.getObject<int>(input_name+"Spill", input_pass);
      ticks = event.getObject<int>(input_name+"Ticks", input_pass);
      digis = event.getObject<ldmx::HgcrocDigiCollection>(input_name,input_pass);
    }
    bool same_event(const PolarfireQueueEntry& other) {
      return spill == other.spill and abs(ticks - other.ticks) < max_tick_diff_;
    }
    bool earlier_event(const PolarfireQueueEntry& other) {
      if (spill != other.spill) return spill < other.spill;
      return ticks < other.ticks;
    }
  };
  /// queue of unmatched digis within a spill
  std::queue<PolarfireQueueEntry> pf0_queue, pf1_queue;
  /// spill we are on currently
  int spill_{-1};
 public:
  HcalAlignPolarfires(const std::string& n, framework::Process& p)
    : framework::Producer(n,p) {}
  virtual ~HcalAlignPolarfires() = default;
  virtual void configure(framework::config::Parameters& ps) final override;
  virtual void produce(framework::Event& event) final override;
};

void HcalAlignPolarfires::configure(framework::config::Parameters& ps) {
  input_names_ = ps.getParameter<std::vector<std::string>>("input_names");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  max_tick_diff_ = ps.getParameter<int>("max_tick_diff");
} // configure

void HcalAlignPolarfires::produce(framework::Event& event) {
  // put next package of decoding into the queues
  pf0_queue.emplace(event, input_names_[0], input_pass_);
  pf1_queue.emplace(event, input_names_[1], input_pass_);

  // remove empty events from front of queues
  while (pf0_queue.size() > 0 and pf0_queue.front().digis.getNumDigis() == 0) pf0_queue.pop();
  while (pf1_queue.size() > 0 and pf1_queue.front().digis.getNumDigis() == 0) pf1_queue.pop();

  // if both queues are empty, abort event
  if (pf0_queue.size() == 0 and pf1_queue.size() == 1) abortEvent();

  // we have non-empty events in the queues, start alignment procedure
  if (pf0_queue.front().same_event(pf1_queue.front())) {
    // add them together and put them into same object
    auto merged = pf0_queue.front().digis;
    const auto& unmerged{pf1_queue.front().digis};
    for (int i{0}; i < unmerged.getNumDigis(); i++) {
      auto digi{unmerged.getDigi(i)};
      std::vector<ldmx::HgcrocDigiCollection::Sample> samples;
      for (int j{0}; j < unmerged.getNumSamplesPerDigi(); j++) {
        samples.push_back(digi.at(i));
      }
      merged.addDigi(digi.id(), samples);
    }
    event.add(output_name_,merged);
    pf0_queue.pop();
    pf1_queue.pop();
    setStorageHint(framework::hint_shouldKeep);
  } else if (pf0_queue.front().earlier_event(pf1_queue.front())) {
    // should add pf0 but signal event is unmerged
    event.add(output_name_, pf0_queue.front().digis);
    pf0_queue.pop();
    setStorageHint(framework::hint_shouldDrop);
  } else {
    // should add pf1 but signal event is unmerged
    event.add(output_name_, pf1_queue.front().digis);
    pf1_queue.pop();
    setStorageHint(framework::hint_shouldDrop);
  }
} // produce

}

DECLARE_PRODUCER_NS(hcal, HcalAlignPolarfires);
