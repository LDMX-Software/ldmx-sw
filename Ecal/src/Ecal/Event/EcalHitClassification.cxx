#include "Ecal/Event/EcalHitClassification.h"

#include <iomanip>
#include <ostream>

ClassImp(ldmx::EcalHitClassification);

namespace ldmx {

void EcalHitClassification::clear() {
  id_ = 0;
  classification_ = -1;
  confidence_ = 0.0f;
  truth_origin_id_ = -1;
  truth_classification_ = -1;
  truth_fraction_ = 0.0f;
  has_truth_ = false;
  is_correctly_classified_ = false;
}

std::ostream& operator<<(std::ostream& output,
                         const EcalHitClassification& result) {
  return output << "EcalHitClassification { id: 0x" << std::hex
                << result.getID() << std::dec
                << ", classification: " << result.getClassification()
                << ", confidence: " << result.getConfidence()
                << ", truth origin: " << result.getTruthOriginID()
                << ", truth classification: "
                << result.getTruthClassification()
                << ", truth fraction: " << result.getTruthFraction()
                << ", has truth: " << result.hasTruth()
                << ", correct: " << result.isCorrectlyClassified() << " }";
}

}  // namespace ldmx
