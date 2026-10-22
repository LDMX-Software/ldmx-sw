#include "SimCore/HepMC/HepMCReader.h"

#include <fstream>
#include <string>

#include "Framework/Exception/Exception.h"
#include "HepMC3/ReaderFactory.h"
#include "HepMC3/ReaderRoot.h"
#include "HepMC3/ReaderRootTree.h"
#include "TFile.h"
#include "TTree.h"

namespace simcore {
namespace hepmc {

HepMCReader::HepMCReader(std::string& filename) {
  ldmx_log(info) << "Opening HepMC file " << filename;

  // ROOT files start with the "root" magic bytes
  std::string magic(4, '\0');
  std::ifstream(filename, std::ios::binary).read(magic.data(), magic.size());
  if (magic == "root") {
    // WriterRootTree stores a TTree, WriterRoot one object per event
    std::unique_ptr<TFile> file{TFile::Open(filename.c_str())};
    if (file && file->Get<TTree>("hepmc3_tree")) {
      ldmx_log(info) << "Detected HepMC3 ROOT tree format";
      reader_ = std::make_shared<HepMC3::ReaderRootTree>(filename);
    } else {
      ldmx_log(info) << "Detected HepMC3 ROOT format";
      reader_ = std::make_shared<HepMC3::ReaderRoot>(filename);
    }
  } else {
    // HepMC2/3 ascii, LHEF or HEPEVT
    reader_ = HepMC3::deduce_reader(filename);
  }

  if (!reader_ || reader_->failed()) {
    EXCEPTION_RAISE("BadFile", "Failed to open HepMC file: " + filename);
  }
}

std::unique_ptr<HepMCEvent> HepMCReader::readNextEvent() {
  // Create a new HepMC3 GenEvent
  auto gen_event = std::make_shared<HepMC3::GenEvent>();

  // Read the next event from the file
  if (!reader_->read_event(*gen_event)) {
    ldmx_log(warn) << "No next event was found by the HepMC reader.";
    return nullptr;
  }

  // Geant4 expects GeV and mm
  gen_event->set_units(HepMC3::Units::GEV, HepMC3::Units::MM);

  // Check if the event is empty
  if (gen_event->particles().empty()) {
    ldmx_log(warn) << "Empty event found by HepMC reader.";
    return nullptr;
  }

  // Increment event counter and log progress every 100 events
  event_counter_++;
  if (event_counter_ % 100 == 0) {
    ldmx_log(debug) << "HepMCReader: Read " << event_counter_ << " events";
  }

  // Create and return the wrapped event
  return std::make_unique<HepMCEvent>(gen_event);
}

}  // namespace hepmc
}  // namespace simcore
