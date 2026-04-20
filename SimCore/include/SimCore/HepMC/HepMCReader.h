/**
 * @file HepMCReader.h
 * @brief Class for reading HepMC event data
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_HEPMCREADER_H_
#define SIMCORE_HEPMCREADER_H_

// LDMX
#include "Framework/Logger.h"
#include "SimCore/HepMC/HepMCEvent.h"

// HepMC3
#include "HepMC3/Reader.h"
#include "HepMC3/ReaderAscii.h"
#include "HepMC3/ReaderAsciiHepMC2.h"

// STL
#include <memory>
#include <string>

namespace simcore::hepmc {

/**
 * @class HepMCReader
 * @brief Reads HepMC event data into a HepMCEvent object
 */
class HepMCReader {
 public:
  /**
   * Class constructor.
   * @param fileName The input file name.
   */
  HepMCReader(std::string& fileName);

  /**
   * Class destructor.
   */
  virtual ~HepMCReader() = default;

  /**
   * Read the next event.
   * @return The next HepMC event.
   */
  std::unique_ptr<HepMCEvent> readNextEvent();

 private:
  /**
   * The HepMC3 reader.
   */
  std::shared_ptr<HepMC3::Reader> reader_;

  /**
   * Event counter for tracking progress.
   */
  mutable int event_counter_{0};

  // enable logging
  enableLogging("HepMCReader")
};

}  // namespace simcore::hepmc

#endif
