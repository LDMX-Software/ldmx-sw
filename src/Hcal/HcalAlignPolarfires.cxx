
#include <queue>

#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"

namespace hcal {

/**
 * Align the two polarfires with drop/keep hints signalling successful merge
 *
 * - Only checking for /dropped/ events
 * - assuming that ticks and spills are already in correct ORDER
 * - assuming spill numbering is NOT the same between the two DPMs
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
    /// the i'th spill
    int spill;
    /// ticks since spill
    int ticks;
    ldmx::HgcrocDigiCollection digis;
    PolarfireQueueEntry(const framework::Event& event,
                        const std::string& input_name,
                        const std::string& input_pass,
                        std::pair<int, int>& spill_counter) {
      int spilln = event.getObject<int>(input_name + "Spill", input_pass);
      if (spilln != spill_counter.second) {
        spill_counter.first++;
        spill_counter.second = spilln;
      }
      spill = spill_counter.first;
      ticks = event.getObject<int>(input_name + "Ticks", input_pass);
      digis =
          event.getObject<ldmx::HgcrocDigiCollection>(input_name, input_pass);
    }
    bool same_event(const PolarfireQueueEntry& rhs) {
      return (spill == rhs.spill and abs(ticks - rhs.ticks) < max_tick_diff_);
    }
    bool earlier_event(const PolarfireQueueEntry& rhs) {
      if (spill == rhs.spill) return ticks < rhs.ticks;
      return spill < rhs.spill;
    }
  };
  /// queue of unmatched digis
  std::queue<PolarfireQueueEntry> pf0_queue, pf1_queue;
  /// spill counter
  std::pair<int, int> pf0_spill_counter{0, -1}, pf1_spill_counter{0, -1};

 public:
  HcalAlignPolarfires(const std::string& n, framework::Process& p)
      : framework::Producer(n, p) {}
  virtual ~HcalAlignPolarfires() = default;
  virtual void configure(framework::config::Parameters& ps) final override;
  virtual void produce(framework::Event& event) final override;
};

int HcalAlignPolarfires::max_tick_diff_ = 10;

void HcalAlignPolarfires::configure(framework::config::Parameters& ps) {
  input_names_ = ps.getParameter<std::vector<std::string>>("input_names");
  input_pass_ = ps.getParameter<std::string>("input_pass");
  output_name_ = ps.getParameter<std::string>("output_name");
  max_tick_diff_ = ps.getParameter<int>("max_tick_diff");
}  // configure

void HcalAlignPolarfires::produce(framework::Event& event) {
  // put next package of decoding into the queues
  pf0_queue.emplace(event, input_names_[0], input_pass_, pf0_spill_counter);
  pf1_queue.emplace(event, input_names_[1], input_pass_, pf1_spill_counter);

  // remove empty events from front of queues for end-of-file condition
  while (pf0_queue.size() > 0 and pf0_queue.front().digis.getNumDigis() == 0)
    pf0_queue.pop();
  while (pf1_queue.size() > 0 and pf1_queue.front().digis.getNumDigis() == 0)
    pf1_queue.pop();

  bool aligned{false};
  ldmx::HgcrocDigiCollection merged;
  if (pf0_queue.size() > 0 and pf1_queue.size() > 0) {
    // we have non-empty events in the queues, start alignment procedure
    if (pf0_queue.front().same_event(pf1_queue.front())) {
      // same spill and ticks within max_tick_diff_ of each other
      // add them together and put them into same object
      merged = pf0_queue.front().digis;
      const auto& unmerged{pf1_queue.front().digis};
      for (int i{0}; i < unmerged.getNumDigis(); i++) {
        auto digi{unmerged.getDigi(i)};
        std::vector<ldmx::HgcrocDigiCollection::Sample> samples;
        for (int j{0}; j < unmerged.getNumSamplesPerDigi(); j++) {
          samples.push_back(digi.at(i));
        }
        merged.addDigi(digi.id(), samples);
      }
      aligned = true;
      pf0_queue.pop();
      pf1_queue.pop();
      setStorageHint(framework::hint_shouldKeep);
    } else if (pf0_queue.front().earlier_event(pf1_queue.front())) {
      // should add pf0 but signal event is unmerged
      merged = pf0_queue.front().digis;
      pf0_queue.pop();
      setStorageHint(framework::hint_shouldDrop);
    } else {
      // should add pf1 but signal event is unmerged
      merged = pf1_queue.front().digis;
      pf1_queue.pop();
      setStorageHint(framework::hint_shouldDrop);
    }
  } else if (pf0_queue.size() > 0) {
    // only pf0 has non-empty events left
    // should add pf0 but signal event is unmerged
    merged = pf0_queue.front().digis;
    pf0_queue.pop();
    setStorageHint(framework::hint_shouldDrop);
  } else if (pf1_queue.size() > 0) {
    // only pf1 has non-empty events left
    // should add pf1 but signal event is unmerged
    merged = pf1_queue.front().digis;
    pf1_queue.pop();
    setStorageHint(framework::hint_shouldDrop);
  } else {
    // no more events, both decoders are returning empty events
    abortEvent();
  }

  event.add(output_name_, merged);
  event.add(output_name_ + "Aligned", aligned);
}  // produce

}  // namespace hcal

DECLARE_PRODUCER_NS(hcal, HcalAlignPolarfires);
