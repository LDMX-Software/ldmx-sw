#include "SimCore/HepMC/HepMCReader.h"

#include <fstream>
#include <string>

namespace simcore {
namespace hepmc {

HepMCReader::HepMCReader(std::string& filename) {
  ldmx_log(info) << "Opening HepMC file " << filename;

  // Try to detect the file format by reading the first non-empty line
  std::ifstream test_file(filename);
  std::string first_line;
  bool found_line = false;

  if (test_file.is_open()) {
    // Skip empty lines and find the first non-empty line
    while (std::getline(test_file, first_line)) {
      // Trim whitespace
      first_line.erase(0, first_line.find_first_not_of(" \t\r\n"));
      first_line.erase(first_line.find_last_not_of(" \t\r\n") + 1);

      if (!first_line.empty()) {
        found_line = true;
        break;
      }
    }
    test_file.close();
  }

  if (found_line) {
    ldmx_log(debug) << "First non-empty line: " << first_line;

    // Check if it's HepMC2 format
    if (first_line.find("HepMC::") != std::string::npos ||
        first_line.find("IO_GenEvent") != std::string::npos) {
      ldmx_log(info) << "Detected HepMC2 format, using ReaderAsciiHepMC2";
      reader_ = std::make_shared<HepMC3::ReaderAsciiHepMC2>(filename);
    } else {
      ldmx_log(info) << "Assuming HepMC3 format, using ReaderAscii";
      reader_ = std::make_shared<HepMC3::ReaderAscii>(filename);
    }
  } else {
    ldmx_log(warn) << "Could not find non-empty line, defaulting to HepMC3 format";
    reader_ = std::make_shared<HepMC3::ReaderAscii>(filename);
  }

  if (reader_->failed()) {
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
