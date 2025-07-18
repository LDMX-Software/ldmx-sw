/**
 * @file LHEReader.h
 * @brief Class for reading LHE event data
 * @author Jeremy McCormick, SLAC National Accelerator Laboratory
 * @author Tamas Almos Vami, UCSB
 */

#ifndef SIMCORE_LHEREADER_H_
#define SIMCORE_LHEREADER_H_

// LDMX
#include "Framework/Logger.h"
#include "SimCore/LHE/LHEEvent.h"

// STL
#include <fstream>
#include <iostream>

namespace simcore::lhe {

/**
 * @class LHEReader
 * @brief Reads LHE event data into an LHEEvent object
 */
class LHEReader {
 public:
  /**
   * Class constructor.
   * @param fileName The input file name.
   */
  LHEReader(std::string& fileName);

  /**
   * Class destructor.
   */
  virtual ~LHEReader() = default;

  /**
   * Read the next event.
   * @return The next LHE event.
   */
  std::unique_ptr<LHEEvent> readNextEvent();

 private:
  /**
   * The input file stream.
   */
  std::ifstream ifs_;

  // enable logging
  enableLogging("LHEReader")
};

}  // namespace simcore::lhe

#endif
