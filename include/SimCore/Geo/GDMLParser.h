#ifndef SIMCORE_GEO_GDMLPARSER_H_
#define SIMCORE_GEO_GDMLPARSER_H_

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4GDMLParser.hh"

/*~~~~~~~~~~~~~~~*/
/*   Framework   */
/*~~~~~~~~~~~~~~~*/
#include "Framework/Configure/Parameters.h"

/*~~~~~~~~~~~~~*/
/*   SimCore   */
/*~~~~~~~~~~~~~*/
#include "SimCore/AuxInfoReader.h"
#include "SimCore/Geo/Parser.h"

// Forward Declarations
class G4VPhysicalVolume;

namespace simcore {
namespace geo {

/**
 */
class GDMLParser : public Parser {

public:
  /// Default constructor
  GDMLParser(ldmx::Parameters &parameters);

  /// Default destructor
  ~GDMLParser();

  /**
   * Retrieve the G4VPhysicalVolume associated with the most top-level
   * (world) volume.
   *
   * @return The world volume.
   */
  G4VPhysicalVolume *GetWorldVolume() final override;

  /**
   * Get the name of the parsed detector.
   *
   * This name is typically extracted from the file containing the detector
   * description.
   *
   * @return The name of the detector.
   */
  std::string getDetectorName() final override;

  /**
   * Parse the detector geometry and read it into memory.
   */
  void read() final override;

  /**
   * Create an instance of this parser.
   */
  static Parser *create(ldmx::Parameters &parameters) {
    return new GDMLParser(parameters);
  }

private:
  /// The GDML parser.
  std::unique_ptr<G4GDMLParser> parser_;

  /// The auxiliary info reader
  std::unique_ptr<ldmx::AuxInfoReader> info_;

  /// The parameters used to configure this parser
  ldmx::Parameters parameters_;

}; // GDMLParser
} // namespace geo
} // namespace simcore

#endif // SIMCORE_GEO_GDMLPARSER_H_
