
#include "Packing/Unpacker.h"

#include "TFile.h"
#include "TTree.h"

namespace packing {

void Unpacker::configure(framework::config::Parameters& ps) {
  // create the configured translators
  Processor::configure(ps);

  raw_file_ = ps.getParameter<std::string>("raw_file");
  raw_tree_ = ps.getParameter<std::string>("raw_tree");
  skip_unavailable_ = ps.getParameter<bool>("skip_unavailable");
}

void Unpacker::onProcessStart() {
  // open file and get tree of raw data
  file_ = TFile::Open(raw_file_.c_str());
  reader_ = new TTreeReader(raw_tree_.c_str(), file_);

  // go through list of branches
  auto branch_listing{reader_->GetTree()->GetListOfBranches()};
  for (int i_br{0}; i_br < branch_listing->GetEntriesFast(); i_br++) {
    auto br{(TBranch*)branch_listing->UncheckedAt(i_br)};
    std::string br_name{br->GetName()};
    auto optional_translator{getTranslator(br_name)};
    if (optional_translator) {
      ldmx_log(info) << "Found a translator for " << br_name;
      unpackers_.emplace_back(*reader_, br_name, *optional_translator);
    } else if (skip_unavailable_) {
      ldmx_log(info) << "Unable to find a translator for '" 
        << br_name << "', skipping...";
    } else {
      EXCEPTION_RAISE("DeduceTranslator",
          "Unable to find a translator that can translate '"+br_name+"'.");
    }
  }
}

void Unpacker::produce(framework::Event& event) {
  if (not reader_->Next()) {
    /// can we turn this into end run?
    abortEvent();
  }

  for (auto& unpacker : unpackers_)
    unpacker.decode(event);
}

void Unpacker::onProcessEnd() {
  std::cout << "Clean-up reader..." << std::endl;
  delete reader_;
  std::cout << "Close file..." << std::endl;
  file_->Close();
  std::cout << "All done" << std::endl;
}

Unpacker::SingleUnpacker::SingleUnpacker(TTreeReader& r, const std::string& br_name, TranslatorPtr t) : translator_{t}, buffer_{r, br_name.c_str()}, br_name_{br_name} {
  std::cout << "SingleUnpacker(" << br_name << ")" << std::endl;
}

void Unpacker::SingleUnpacker::decode(framework::Event& e) {
  translator_->decode(e, *buffer_);
}

}  // namespace packing

DECLARE_PRODUCER_NS(packing, Unpacker)
