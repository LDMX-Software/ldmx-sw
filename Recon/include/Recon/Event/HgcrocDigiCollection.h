/**
 * @file HgcrocDigiCollection.h
 * @brief Class that represents a digitized hit in a calorimeter cell readout by
 * an HGCROC
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#ifndef RECON_EVENT_HGCROCDIGICOLLECTION_H_
#define RECON_EVENT_HGCROCDIGICOLLECTION_H_

// ROOT
#include "TObject.h"  //for ClassDef

// STL
#include <stdint.h>  //32bit words
#include <iostream>  //Print method
#include <vector>    //vector lists

namespace ldmx {

/**
 * @class HgcrocDigiCollection
 * @brief Represents a collection of the digi hits readout by an HGCROC
 *
 * @note This class represents the digitized signal information
 * in the form of a series of samples for each channel of readout.
 * Each channel is represented by an ID integer and each sample is a 32-bit
 * word. The number of samples for each digi is configurable, but is required to
 * be the same for all channels.
 *
 * Each digi corresponds to one channel ID and numSamplesPerDigi_ samples.
 */
class HgcrocDigiCollection {
 public:
  /**
   * @class Sample
   * @brief One sample of a digi channel corresponding to one clock of the
   * HGCROC chip
   *
   * Not all of these measurements are valid in each sample.
   * The valid measurements depend on the tot_progress and tot_complete flags.
   *
   * The TOA measurement is always valid and is inserted as the third
   * measurement in the 32-bit word.
   *
   * If the TOT measurment is NOT complete, then the other
   * two valid measurements (in order) are
   *  1. ADC of the previous sample (adc_tm1)
   *  2. ADC of this sample (adc_t)
   *
   * If the TOT is NOT in progress and the TOT is complete, then
   *  1. ADC of the previous sample (adc_tm1)
   *  2. TOT measurement (tot)
   *
   * If both flags are true, then
   *  1. ADC of this sample (adc_t)
   *  2. TOT measurement (tot)
   *
   * Usually several samples are used for each channel to re-construct the hit.
   */
  class Sample {
   public:
    /**
     * Helpful alternative constructor
     *
     * Encodes the various measurements into the word depending on the passed
     * flags. Use this constructor inside of the chip emulator when converting
     * voltage pulses into DIGIs.
     *
     * @note This is where the measurements to word translation occurs.
     */
    Sample(bool tot_progress, bool tot_complete, int firstMeas, int seconMeas,
           int toa);

    /**
     * Basic constructor
     *
     * Use this constructor when translating binary data coming off the
     * detector into our event model.
     */
    Sample(uint32_t w) : word_(w) {}

    /**
     * Default constructor
     *
     * Not used, but required for Event dictionary generation
     */
    Sample() {}

    /**
     * Get the first flag from the sample
     * checking if TOT is in progress during this sample
     *
     * @return true if TOT is in progress during this sample
     */
    bool isTOTinProgress() const {
      return (ONE_BIT_MASK & (word_ >> FIRSTFLAG_POS));
    }

    /**
     * Get the second flag from the sample
     * checking if TOT is complete at this sample
     *
     * @return true if TOT is complete and should use this sample to get TOT
     * measurement
     */
    bool isTOTComplete() const {
      return (ONE_BIT_MASK & (word_ >> SECONFLAG_POS));
    }

    /**
     * Get the Time Of Arrival of this sample
     * which is always the third position in all readout modes.
     *
     * @return 10-bit measurement of TOA
     */
    int toa() const { return (TEN_BIT_MASK & word_); }

    /**
     * Get the TOT measurement from this sample
     *
     * @note Does not check if this is the TOT Complete sample!
     *
     * Expands the 10-bit measurment inside the sample into
     * the 12-bit actual measurement of TOT.
     *
     * @return 12-bit measurement of TOT
     */
    int tot() const {
      int seconMeas = secon();
      if (seconMeas > 512) seconMeas = (seconMeas - 512) * 8;
      return seconMeas;
    }

    /**
     * Get the last ADC measurement from this sample
     *
     * @note Does not check if this sample has a valid ADC t-1 measurement.
     *
     * @return 10-bit measurement of ADC t-1
     */
    int adc_tm1() const { return first(); }

    /**
     * Get the ADC measurement from this sample
     *
     * Checks which running mode we are in to determine
     * which position the measurement should be taken from.
     *
     * @return 10-bit measurement of current ADC
     */
    int adc_t() const {
      if (not isTOTComplete())
        return secon();  // running modes
      else
        return first();  // calibration mode
    }

    /**
     * Get the raw value of this sample
     *
     * @return 32-bit full value fo the sample
     */
    uint32_t raw() const { return word_; }

   private:
    /**
     * Get the first 10-bit measurement out of the sample
     *
     * @return 10-bit measurement at first position in sample
     */
    int first() const { return TEN_BIT_MASK & (word_ >> FIRSTMEAS_POS); }

    /**
     * Get the second 10-bit measurement out of the sample
     *
     * @return 10-bit measurement at second position in sample
     */
    int secon() const { return TEN_BIT_MASK & (word_ >> SECONMEAS_POS); }

   private:
    /// The actual 32-bit word spit out by the chip
    uint32_t word_;

  };  // Sample

 public:
  /**
   * @class HgcrocDigi
   * One DIGI signal coming from the HGC ROC
   *
   * This stores the channel ID and the samples coming from a channel
   * It really only makes sense to use this object when retrieving DIGIs
   * from the collection. Do not try to make one of these yourself.
   */
  class HgcrocDigi {
   public:
    /**
     * Constructor
     *
     * Passes required variables to this structure.
     *
     * @param[in] id global integer ID for the DIGI channel
     * @param[in] first iterator pointing to first sample of this DIGI in the
     * collection
     * @param[in] collection const reference to the collection this DIGI
     * references
     */
    HgcrocDigi(unsigned int id,
               std::vector<HgcrocDigiCollection::Sample>::const_iterator first,
               const HgcrocDigiCollection& collection)
        : id_(id), first_(first), collection_(collection) {}

    /**
     * Get the ID for this DIGI
     *
     * @return global integer ID for this DIGI channel
     */
    unsigned int id() const { return id_; }

    /**
     * Get the starting iterator for looping through this DIGI
     *
     * @return const vector iterator pointing to start of this DIGI
     */
    auto begin() const { return first_; }

    /**
     * Get the ending iterator for looping through this DIGI
     *
     * @return const vector iterator pointing to end of this DIGI
     */
    auto end() const { return first_ + collection_.getNumSamplesPerDigi(); }

    /**
     * Check if this DIGI is an ADC measurement
     *
     * We consider this DIGI an ADC measurement if both of the flags
     * for the Sample Of Interest are false.
     *
     * @return true if we consider this DIGI an ADC measurement
     */
    bool isADC() const {
      return !(soi().isTOTinProgress() or soi().isTOTComplete());
    }

    /**
     * Check if this DIGI is a TOT measurement
     *
     * @note Just NOT an ADC measurement right now.
     * May need to include the callibration case in the future.
     *
     * @return true if this DIGI is a TOT measurement
     */
    bool isTOT() const { return !isADC(); }

    /**
     * Get the 12-bit decoded TOT measurement from this DIGI
     *
     * @note Returns -1 if this DIGI is not TOT.
     * @note Returns -2 if this DIGI has a TOT in progress during
     * the SOI.
     *
     * @return 12-bit TOT measurement
     */
    int tot() const {
      if (not isTOT()) return -1;
      if (soi().isTOTinProgress()) return -2;
      return soi().tot();
    }

    /**
     * Get the sample of interest from this DIGI
     *
     * @return Sample that is the sample of interest in this DIGI
     */
    const HgcrocDigiCollection::Sample& soi() const {
      return *(first_ + collection_.getSampleOfInterestIndex());
    }

   private:
    /// channel ID where this signal is coming from
    unsigned int id_;

    /// the location of the first sample that are in this digi
    std::vector<HgcrocDigiCollection::Sample>::const_iterator first_;

    /// Reference to collection that owns this DIGI
    const HgcrocDigiCollection& collection_;

  };  // HgcrocDigi

 public:
  /**
   * Class constructor.
   */
  HgcrocDigiCollection() {}

  /**
   * Class destructor.
   */
  virtual ~HgcrocDigiCollection() {}

  /**
   * Clear the data in the object.
   *
   * Clears the vectors of channel IDs and samples,
   * but does not change the other settings of this collection.
   */
  void Clear();

  /**
   * Print out the object.
   *
   * Prints out the lengths of the stored vectors and
   * the other settings of this collection.
   */
  void Print() const;

  /**
   * Get number of samples per digi
   * @return unsigned int number of samples per digi
   */
  unsigned int getNumSamplesPerDigi() const { return numSamplesPerDigi_; }

  /**
   * Set number of samples for each digi
   * @param[in] n number of samples per digi
   */
  void setNumSamplesPerDigi(unsigned int n) {
    numSamplesPerDigi_ = n;
    return;
  }

  /**
   * Get index of sample of interest
   * @return unsigned int index for SOI
   */
  unsigned int getSampleOfInterestIndex() const { return sampleOfInterest_; }

  /**
   * Set index of sample of interest
   *
   * @note Does not check if input is a valid index!
   * (i.e. input less than numSamplesPerDigi_)
   *
   * @param[in] n index for the sample of interest
   */
  void setSampleOfInterestIndex(unsigned int n) {
    sampleOfInterest_ = n;
    return;
  }

  /**
   * Get samples for the input digi index
   *
   * Each "digi" is numSamplesPerDigi_ samples.
   * The sample is a single 32-bit word that is then translated into
   * the 10-bit measurements depending on the first two bits.
   *
   * @sa Sample for how the valid measurements depend on the flags.
   * @sa HgcrocDigi for how to use the object returned from this function
   *
   * @param[in] digiIndex index of digi to decode
   * @return HgcrocDigi package with accessors
   */
  const HgcrocDigi getDigi(unsigned int digiIndex) const;

  /**
   * Get total number of digis
   * @return unsigned int number of digis
   */
  unsigned int getNumDigis() const { return channelIDs_.size(); }

  /**
   * Get total number of digis
   * @return unsigned int number of digis
   */
  unsigned int size() const { return channelIDs_.size(); }

  /**
   * Add samples to collection
   *
   * @sa Sample for how the valid measurements depend on the flags.
   *
   * @param[in] id global integer ID for this channel
   * @param[in] digi list of new samples to add
   */
  void addDigi(unsigned int id, const std::vector<Sample>& digi);

 public:
  /**
   * iterator class so we can do range-based loops over digi collecitons
   */
  class iterator : public std::iterator<std::input_iterator_tag, HgcrocDigi, long> {
   public:
    explicit iterator(HgcrocDigiCollection& c, long index = 0) : coll_{c}, digi_index_{index} {}
    iterator& operator++() { digi_index_++; return *this; }
    iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
    bool operator==(iterator other) const {return digi_index_ == other.digi_index_; }
    bool operator!=(iterator other) const {return !(*this == other); }
    const HgcrocDigi operator*() const { return coll_.getDigi(digi_index_); }
   private:
    long digi_index_{0};
    HgcrocDigiCollection& coll_;
  };

 public:
  /**
   * The beginning of this colleciton.
   */
  iterator begin() {return iterator(*this,0);}

  /**
   * The end of this collection
   */
  iterator end() {return iterator(*this,getNumDigis()); }

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
  std::vector<unsigned int> channelIDs_;

  /** list of samples that we have been given */
  std::vector<Sample> samples_;

  /** number of samples for each digi */
  unsigned int numSamplesPerDigi_;

  /** index for the sample of interest in the samples list */
  unsigned int sampleOfInterest_;

  /**
   * The ROOT class definition.
   */
  ClassDef(HgcrocDigiCollection, 2);
};
}  // namespace ldmx

/**
 * Streamer for the HgcrocDigiCollection::Sample
 *
 * Prints Flags and three ten bit measurements.
 *
 * @param[in] s ostream to print sample to
 * @param[in] sample Sample to print
 * @return modified ostream
 */
std::ostream& operator<<(std::ostream& s,
                         const ldmx::HgcrocDigiCollection::Sample& sample);

/**
 * Streamer for HgcrocDigiCollection::HgcrocDigi
 *
 * Prints TOT measurement (if it was TOT) or SOI otherwise
 *
 * @param[in] s ostream to print sample to
 * @param[in] digi HgcrocDigi to print
 * @return modified ostream
 */
std::ostream& operator<<(std::ostream& s,
                         const ldmx::HgcrocDigiCollection::HgcrocDigi& digi);

/**
 * Streamer for HgcrocDigiCollection
 *
 * Prints all of the digi's using their streamer.
 *
 * @param[in] s ostream to print sample to
 * @param[in] col HgcrocDigiCollection to print
 * @return modified ostream
 */
std::ostream& operator<<(std::ostream& s,
                         const ldmx::HgcrocDigiCollection& col);

#endif /* RECON_EVENT_ECALDIGI_H_ */
