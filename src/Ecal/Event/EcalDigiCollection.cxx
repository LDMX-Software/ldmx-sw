/**
 * @file EcalDigiCollection.cxx
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/Event/EcalDigiCollection.h"

ClassImp(ldmx::EcalDigiCollection)

    namespace ldmx {
  void EcalDigiCollection::Clear() {
    channelIDs_.clear();
    samples_.clear();

    return;
  }

  void EcalDigiCollection::Print() const {
    std::cout << "EcalDigiCollection { Num Channel IDs: " << channelIDs_.size()
              << ", Num Samples: " << samples_.size()
              << ", Samples Per Digi: " << numSamplesPerDigi_
              << ", Index for SOI: " << sampleOfInterest_ << "}" << std::endl;

    return;
  }

  std::vector<EcalDigiSample> EcalDigiCollection::getDigi(
      unsigned int digiIndex) const {
    std::vector<EcalDigiSample> digi;
    for (unsigned int sampleIndex = 0;
         sampleIndex < this->getNumSamplesPerDigi(); sampleIndex++) {
      EcalDigiSample sample;

      sample.rawID_ = channelIDs_.at(digiIndex);

      int32_t word = samples_.at(digiIndex * numSamplesPerDigi_ + sampleIndex);

      // this is where the word --> measurements translation occurs

      bool firstFlag = ONE_BIT_MASK & (word >> FIRSTFLAG_POS);
      bool seconFlag = ONE_BIT_MASK & (word >> SECONFLAG_POS);
      int firstMeas = TEN_BIT_MASK & (word >> FIRSTMEAS_POS);
      int seconMeas = TEN_BIT_MASK & (word >> SECONMEAS_POS);
      int lastMeas = TEN_BIT_MASK & (word);

      // the chip returns flags that determine what the three measurements are
      //  I (Tom E) don't know right now what that mapping is, so I will not use
      //  them.

      sample.adc_t_ = firstMeas;
      sample.tot_ = seconMeas;
      sample.toa_ = lastMeas;
      sample.adc_tm1_ = -99;

      digi.push_back(sample);
    }

    return digi;
  }

  void EcalDigiCollection::addDigi(std::vector<EcalDigiSample> newSamples) {
    if (newSamples.size() != this->getNumSamplesPerDigi()) {
      std::cerr
          << "[ WARN ] [ EcalDigiCollection ] Input list of samples has size '"
          << newSamples.size()
          << "' that does not match the number of samples per digi '"
          << this->getNumSamplesPerDigi() << "'!." << std::endl;
      return;
    }

    int channelID = newSamples.at(0).rawID_;
    channelIDs_.push_back(channelID);

    for (auto const &sample : newSamples) {
      int32_t word;

      // this is where the measurements --> word translation occurs

      // check if over largest number possible ==> set to largest if over
      //  don't want wrapping
      int adc_t = (sample.adc_t_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.adc_t_;
      int tot = (sample.tot_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.tot_;
      int toa = (sample.toa_ > TEN_BIT_MASK) ? TEN_BIT_MASK : sample.toa_;

      // the chip returns flags that determine what the three measurements are
      //  I (Tom E) don't know right now what that mapping is, so I will not use
      //  them. Just hard-coding ADCt, TOT, and TOA right now

      word = (1 << FIRSTFLAG_POS) + (1 << SECONFLAG_POS) +
             ((adc_t & TEN_BIT_MASK) << FIRSTMEAS_POS) +
             ((tot & TEN_BIT_MASK) << SECONMEAS_POS) + (toa & TEN_BIT_MASK);

      samples_.push_back(word);
    }

    return;
  }

}  // ldmx
