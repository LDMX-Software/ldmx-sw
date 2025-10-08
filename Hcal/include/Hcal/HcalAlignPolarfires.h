#ifndef HCALALIGNPOLARFIRES_H
#define HCALALIGNPOLARFIRES_H
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
  /// input decoded objects (vector index == polarfire index_)
  std::vector<std::string> input_names_;
  /// pass name for decoded objects
  std::string input_pass_;
  /// output object name
  std::string output_name_;
  /// number of 5MHz ticks difference to consider polarfires aligned
  static int max_tick_diff;

 public:
  struct PolarfireQueueEntry {
    /// the i'th spill
    int spill_;
    /// ticks since spill
    int ticks_;
    ldmx::HgcrocDigiCollection digis_;
    PolarfireQueueEntry(const framework::Event& event,
                        const std::string& input_name,
                        const std::string& input_pass,
                        std::pair<int, int>& spill_counter);
    bool sameEvent(const PolarfireQueueEntry& rhs) {
      return (spill_ == rhs.spill_ and
              abs(ticks_ - rhs.ticks_) < max_tick_diff);
    }
    bool earlierEvent(const PolarfireQueueEntry& rhs) {
      if (spill_ == rhs.spill_) return ticks_ < rhs.ticks_;
      return spill_ < rhs.spill_;
    }
  };
  /// queue of unmatched digis
  std::queue<PolarfireQueueEntry> pf0_queue_, pf1_queue_;
  /// spill counter
  std::pair<int, int> pf0_spill_counter_{0, -1}, pf1_spill_counter_{0, -1};

 public:
  HcalAlignPolarfires(const std::string& n, framework::Process& p)
      : framework::Producer(n, p) {}
  virtual ~HcalAlignPolarfires() = default;
  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;
};

}  // namespace hcal
#endif /* HCALALIGNPOLARFIRES_H */
