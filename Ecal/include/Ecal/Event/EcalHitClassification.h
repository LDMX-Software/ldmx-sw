#ifndef ECAL_EVENT_ECALHITCLASSIFICATION_H_
#define ECAL_EVENT_ECALHITCLASSIFICATION_H_

#include <iosfwd>

#include "TObject.h"

namespace ldmx {

/** Prediction and optional simulation truth for one reconstructed ECal hit. */
class EcalHitClassification {
 public:
  EcalHitClassification() = default;
  virtual ~EcalHitClassification() = default;

  void clear();

  int getID() const { return id_; }
  void setID(int id) { id_ = id; }

  int getClassification() const { return classification_; }
  void setClassification(int classification) {
    classification_ = classification;
  }

  float getConfidence() const { return confidence_; }
  void setConfidence(float confidence) { confidence_ = confidence; }

  int getTruthOriginID() const { return truth_origin_id_; }
  void setTruthOriginID(int origin_id) { truth_origin_id_ = origin_id; }

  int getTruthClassification() const { return truth_classification_; }
  void setTruthClassification(int classification) {
    truth_classification_ = classification;
  }

  float getTruthFraction() const { return truth_fraction_; }
  void setTruthFraction(float fraction) { truth_fraction_ = fraction; }

  bool hasTruth() const { return has_truth_; }
  void setHasTruth(bool has_truth) { has_truth_ = has_truth; }

  bool isCorrectlyClassified() const { return is_correctly_classified_; }
  void setCorrectlyClassified(bool correct) {
    is_correctly_classified_ = correct;
  }

  friend std::ostream& operator<<(std::ostream& output,
                                  const EcalHitClassification& result);

 private:
  int id_{0};
  int classification_{-1};
  float confidence_{0.0f};

  int truth_origin_id_{-1};
  int truth_classification_{-1};
  float truth_fraction_{0.0f};
  bool has_truth_{false};
  bool is_correctly_classified_{false};

  ClassDef(EcalHitClassification, 3);
};

}  // namespace ldmx

#endif  // ECAL_EVENT_ECALHITCLASSIFICATION_H_
