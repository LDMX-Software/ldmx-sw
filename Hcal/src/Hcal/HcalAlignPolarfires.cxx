
#include "Hcal/HcalAlignPolarfires.h"
namespace hcal {
int HcalAlignPolarfires::max_tick_diff = 10;
HcalAlignPolarfires::PolarfireQueueEntry::PolarfireQueueEntry(
    const framework::Event& event, const std::string& input_name,
    const std::string& input_pass, std::pair<int, int>& spill_counter) {
  int spilln = event.getObject<int>(input_name + "Spill", input_pass);
  if (spilln != spill_counter.second) {
    spill_counter.first++;
    spill_counter.second = spilln;
  }
  spilln = spill_counter.first;
  ticks_ = event.getObject<int>(input_name + "Ticks", input_pass);
  digis_ = event.getObject<ldmx::HgcrocDigiCollection>(input_name, input_pass);
}

void HcalAlignPolarfires::configure(framework::config::Parameters& ps) {
  input_names_ = ps.get<std::vector<std::string>>("input_names");
  input_pass_ = ps.get<std::string>("input_pass");
  output_name_ = ps.get<std::string>("output_name");
  max_tick_diff = ps.get<int>("max_tick_diff");
}  // configure

void HcalAlignPolarfires::produce(framework::Event& event) {
  // put next package of decoding into the queues
  pf0_queue_.emplace(event, input_names_[0], input_pass_, pf0_spill_counter_);
  pf1_queue_.emplace(event, input_names_[1], input_pass_, pf1_spill_counter_);

  // remove empty events from front of queues for end-of-file condition
  while (pf0_queue_.size() > 0 and pf0_queue_.front().digis_.getNumDigis() == 0)
    pf0_queue_.pop();
  while (pf1_queue_.size() > 0 and pf1_queue_.front().digis_.getNumDigis() == 0)
    pf1_queue_.pop();

  bool aligned{false};
  ldmx::HgcrocDigiCollection merged;
  if (pf0_queue_.size() > 0 and pf1_queue_.size() > 0) {
    // we have non-empty events in the queues, start alignment procedure
    if (pf0_queue_.front().sameEvent(pf1_queue_.front())) {
      // same spill and ticks within max_tick_diff_ of each other
      // add them together and put them into same object
      merged = pf0_queue_.front().digis_;
      const auto& unmerged{pf1_queue_.front().digis_};
      for (int i{0}; i < unmerged.getNumDigis(); i++) {
        auto digi{unmerged.getDigi(i)};
        std::vector<ldmx::HgcrocDigiCollection::Sample> samples;
        for (int j{0}; j < unmerged.getNumSamplesPerDigi(); j++) {
          samples.push_back(digi.at(i));
        }
        merged.addDigi(digi.id(), samples);
      }
      aligned = true;
      pf0_queue_.pop();
      pf1_queue_.pop();
      setStorageHint(framework::HINT_SHOULD_KEEP);
    } else if (pf0_queue_.front().earlierEvent(pf1_queue_.front())) {
      // should add pf0 but signal event is unmerged
      merged = pf0_queue_.front().digis_;
      pf0_queue_.pop();
      setStorageHint(framework::HINT_SHOULD_DROP);
    } else {
      // should add pf1 but signal event is unmerged
      merged = pf1_queue_.front().digis_;
      pf1_queue_.pop();
      setStorageHint(framework::HINT_SHOULD_DROP);
    }
  } else if (pf0_queue_.size() > 0) {
    // only pf0 has non-empty events left
    // should add pf0 but signal event is unmerged
    merged = pf0_queue_.front().digis_;
    pf0_queue_.pop();
    setStorageHint(framework::HINT_SHOULD_DROP);
  } else if (pf1_queue_.size() > 0) {
    // only pf1 has non-empty events left
    // should add pf1 but signal event is unmerged
    merged = pf1_queue_.front().digis_;
    pf1_queue_.pop();
    setStorageHint(framework::HINT_SHOULD_DROP);
  } else {
    // no more events, both decoders are returning empty events
    abortEvent();
  }

  event.add(output_name_, merged);
  event.add(output_name_ + "Aligned", aligned);
}  // produce
}  // namespace hcal

DECLARE_PRODUCER(hcal::HcalAlignPolarfires);
