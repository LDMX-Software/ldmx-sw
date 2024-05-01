#include "SimCore/Geo/GDMLParser.h"

namespace simcore {
namespace geo {

GDMLParser::GDMLParser(framework::config::Parameters &parameters,
                       simcore::ConditionsInterface &ci) {
  detector_ = parameters.getParameter<std::string>("detector");
  validate_ = parameters.getParameter<bool>("validate_detector");
  parser_ = std::make_unique<G4GDMLParser>();
  parser_->SetOverlapCheck(validate_);
  info_ =
      std::make_unique<simcore::geo::AuxInfoReader>(parser_.get(), parameters);
}

G4VPhysicalVolume *GDMLParser::GetWorldVolume() {
  return parser_->GetWorldVolume();
}

void GDMLParser::read() {
  parser_->Read(detector_, validate_);
  info_->readGlobalAuxInfo();
  info_->assignAuxInfoToVolumes();
  detector_name_ = info_->getDetectorHeader()->getName();
}

}  // namespace geo
}  // namespace simcore
