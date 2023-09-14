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
                        std::pair<int, int>& spill_counter);
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
  void configure(framework::config::Parameters& ps) override;
  void produce(framework::Event& event) override;
};

}  // namespace hcal
#endif /* HCALALIGNPOLARFIRES_H */
