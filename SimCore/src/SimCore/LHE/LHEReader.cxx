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
  bool foundEventElement = false;
  while (getline(ifs_, line)) {
    if (line == "<event>") {
      foundEventElement = true;
      break;
    }
  }

  if (!foundEventElement) {
    ldmx_log(warn) << "No next <event> element was found by the LHE reader.";
    return nullptr;
  }

  getline(ifs_, line);

  // Create the LHEEvent using std::make_unique
  auto nextEvent = std::make_unique<LHEEvent>(line);

  while (getline(ifs_, line)) {
    if (line == "</event>" || line == "<mgrwt>") {
      // break if the event ended or in LHE 3.0 if we reach the mgrwt block
      break;
    }

    if (line.find("#") == std::string::npos) {  // not a comment line
      // Create LHEParticle using std::make_unique and add it to the event
      auto particle = std::make_unique<LHEParticle>(line);
      nextEvent->addParticle(std::move(particle));
    } else {
      if (line.find("#vertex") != std::string::npos) {
        nextEvent->setVertex(line);
      }
    }
  }

  const std::vector<std::unique_ptr<LHEParticle>>& particles =
      nextEvent->getParticles();
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

  return nextEvent;
}

}  // namespace lhe
}  // namespace simcore