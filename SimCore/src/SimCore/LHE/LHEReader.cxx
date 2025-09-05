#include "SimCore/LHE/LHEReader.h"

namespace simcore {
namespace lhe {

LHEReader::LHEReader(std::string& filename) {
  ldmx_log(info) << "Opening LHE file " << filename;
  ifs_.open(filename.c_str(), std::ifstream::in);

  if (!ifs_.is_open()) {
    EXCEPTION_RAISE("BadFile", "Failed to open LHE file: " + filename);
  }
}

std::unique_ptr<LHEEvent> LHEReader::readNextEvent() {
  std::string line;
  bool found_event_element = false;
  while (getline(ifs_, line)) {
    auto back =
        std::find_if_not(line.rbegin(), line.rend(), [](unsigned char c) {
          return std::isspace(c);
        }).base();
    line.erase(back, line.end());
    if (line == "<event>") {
      found_event_element = true;
      break;
    }
  }

  if (!found_event_element) {
    ldmx_log(warn) << "No next <event> element was found by the LHE reader.";
    return nullptr;
  }

  getline(ifs_, line);

  // Create the LHEEvent using std::make_unique
  auto next_event = std::make_unique<LHEEvent>(line);

  while (getline(ifs_, line)) {
    auto back =
        std::find_if_not(line.rbegin(), line.rend(), [](unsigned char c) {
          return std::isspace(c);
        }).base();
    line.erase(back, line.end());
    if (line == "</event>" || line == "<mgrwt>") {
      // break if the event ended or in LHE 3.0 if we reach the mgrwt block
      break;
    }

    if (line.find("#") == std::string::npos) {  // not a comment line
      // Create LHEParticle using std::make_unique and add it to the event
      auto particle = std::make_unique<LHEParticle>(line);
      next_event->addParticle(std::move(particle));
    } else {
      if (line.find("#vertex") != std::string::npos) {
        next_event->setVertex(line);
      }
    }
  }

  const std::vector<std::unique_ptr<LHEParticle>>& particles =
      next_event->getParticles();
  for (const auto& particle : particles) {
    if (particle->getMother(0) != 0) {
      int mother1 = particle->getMother(0);
      int mother2 = particle->getMother(1);
      if (mother1 > 0) {
        particle->setMother(0, particles[mother1 - 1].get());
      }
      if (mother2 > 0) {
        particle->setMother(1, particles[mother2 - 1].get());
      }
    }
  }

  return next_event;
}

}  // namespace lhe
}  // namespace simcore