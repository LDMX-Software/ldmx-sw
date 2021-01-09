/**
 * @file EcalDigiCollection.h
 * @brief Class that represents a digitized hit in a calorimeter cell within the
 * ECal
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef EVENT_ECALDIGICOLLECTION_H_
#define EVENT_ECALDIGICOLLECTION_H_

// ROOT
#include "TObject.h"  //for ClassDef

// STL
#include <stdint.h>  //32bit words
#include <iostream>  //Print method
#include <vector>    //vector lists

namespace ecal {
namespace event {

/**
 * @struct EcalDigiSample
 * @brief One sample of an Ecal digi channel
 *
 * Usually several samples are used for each channel to re-construct the hit.
 */
struct EcalDigiSample {
  /** Raw integer ID of channel this sample is for */
  int rawID_{-1};

  /** ADC counts in this channel at this time */
  int adc_t_{0};

  /** ADC counts in this channel at the previous time */
  int adc_tm1_{0};

  /** Time counts over threshhold in this channel */
  int tot_{0};

  /** Time counts when signal arrived in this channel */
  int toa_{0};
};

/**
 * @class EcalDigiCollection
 * @brief Represents a collection of the ECal digi hits
 *
 * @note This class represents the digitized signal information
 * in the form of a series of samples for each channel of readout.
 * Each channel is represented by an ID integer and each sample is a 32-bit
 * word. The number of samples for each digi is configurable, but is required to
 * be the same for all channels.
 *
 * Each digi corresponds to a one channel ID and numSamplesPerDigi_ samples.
 */
class EcalDigiCollection {
 public:
  /**
   * Class constructor.
   */
  EcalDigiCollection() {}

  /**
   * Class destructor.
   */
  virtual ~EcalDigiCollection() {}

  /**
   * Clear the data in the object.
   *
   * Clears the vectors of channel IDs and samples, but does not change the
   * number of samples per digi setting.
   */
  void Clear();

  /**
   * Print out the object.
   *
   * Prints out the lengths of the stored vectors and the number of samples per
   * digi setting.
   */
  void Print() const;

  /**
   * Get number of samples per digi
   */
  unsigned int getNumSamplesPerDigi() const { return numSamplesPerDigi_; }

  /**
   * Set number of samples for each digi
   */
  void setNumSamplesPerDigi(unsigned int n) {
    numSamplesPerDigi_ = n;
    return;
  }

  /**
   * Get index of sample of interest
   */
  unsigned int getSampleOfInterestIndex() const { return sampleOfInterest_; }

  /**
   * Set index of sample of interest
   */
  void setSampleOfInterestIndex(unsigned int n) {
    sampleOfInterest_ = n;
    return;
  }

  /**
   * Get samples for the input digi index
   */
  std::vector<EcalDigiSample> getDigi(unsigned int digiIndex) const;

  /**
   * Get total number of digis
   */
  unsigned int getNumDigis() const { return channelIDs_.size(); }

  /**
   * Translate and add samples to collection
   */
  void addDigi(std::vector<EcalDigiSample> newSamples);

 private:
  /** Mask for lowest order bit in an int */
  static const int ONE_BIT_MASK = 1;

  /** Mask for lowest order ten bits in an int */
  static const int TEN_BIT_MASK = (1 << 10) - 1;

  /** Bit position of first flag */
  static const int FIRSTFLAG_POS = 31;

  /** Bit position of second flag */
  static const int SECONFLAG_POS = 30;

  /** Bit position of first measurement */
  static const int FIRSTMEAS_POS = 20;

  /** Bit position of second measurement */
  static const int SECONMEAS_POS = 10;

 private:
  /** list of channel IDs that we have digis for */
  std::vector<int> channelIDs_;

  /** list of samples that we have been given */
  std::vector<int32_t> samples_;

  /** number of samples for each digi */
  unsigned int numSamplesPerDigi_{1};

  /** index for the sample of interest in the samples list */
  unsigned int sampleOfInterest_{0};

  /**
   * The ROOT class definition.
   */
  ClassDef(EcalDigiCollection, 1);
};
}
}  // namespace ecal

#endif /* EVENT_ECALDIGI_H_ */
