
#include "Recon/Event/HgcrocDigiCollection.h"

ClassImp(ldmx::HgcrocDigiCollection)

    namespace ldmx {
  HgcrocDigiCollection::Sample::Sample(bool tot_progress, bool tot_complete,
                                       int firstMeas, int seconMeas, int toa) {
    if (not tot_progress and tot_complete) {
      // the 12 bit internal tot measurement needs to
      //  be packed into a 10 bit int
      if (seconMeas > 512)
        seconMeas =
            512 + seconMeas / 8;  // lost some precision but can go higher
    }

    // check if over largest number possible ==> set to largest if over (don't
    // want wrapping) and then do bit shifting nonsense to code the measurements
    // into the 32-bit word set last measurement to TOA
    word_ =
        (tot_progress << FIRSTFLAG_POS) + (tot_complete << SECONFLAG_POS) +
        (((firstMeas > TEN_BIT_MASK ? TEN_BIT_MASK : firstMeas) & TEN_BIT_MASK)
         << FIRSTMEAS_POS) +
        (((seconMeas > TEN_BIT_MASK ? TEN_BIT_MASK : seconMeas) & TEN_BIT_MASK)
         << SECONMEAS_POS) +
        (((toa > TEN_BIT_MASK ? TEN_BIT_MASK : toa) & TEN_BIT_MASK));
  }

  void HgcrocDigiCollection::Clear() {
    channelIDs_.clear();
    samples_.clear();

    return;
  }

  void HgcrocDigiCollection::Print() const {
    std::cout << "HgcrocDigiCollection { Num Channel IDs: "
              << channelIDs_.size() << ", Num Samples: " << samples_.size()
              << ", Samples Per Digi: " << numSamplesPerDigi_
              << ", Index for SOI: " << sampleOfInterest_ << "}" << std::endl;

    return;
  }

  const HgcrocDigiCollection::HgcrocDigi HgcrocDigiCollection::getDigi(
      unsigned int digiIndex) const {
    return HgcrocDigiCollection::HgcrocDigi(
        channelIDs_.at(digiIndex),
        samples_.begin() + digiIndex * getNumSamplesPerDigi(), *this);
  }

  void HgcrocDigiCollection::addDigi(
      unsigned int id, const std::vector<HgcrocDigiCollection::Sample> &digi) {
    if (digi.size() != this->getNumSamplesPerDigi()) {
      std::cerr << "[ WARN ] [ HgcrocDigiCollection ] Input list of samples "
                   "has size '"
                << digi.size()
                << "' that does not match the number of samples per digi '"
                << this->getNumSamplesPerDigi() << "'!." << std::endl;
      return;
    }

    channelIDs_.push_back(id);
    for (auto const &s : digi) samples_.push_back(s);

    return;
  }

}  // namespace ldmx

std::ostream &operator<<(std::ostream &s,
                         const ldmx::HgcrocDigiCollection::Sample &sample) {
  s << "Sample { "
    << "tot prog: " << sample.isTOTinProgress() << ", "
    << "tot comp: " << sample.isTOTComplete() << ", ";
  if (sample.isTOTComplete() and sample.isTOTinProgress())
    s << "adc t: " << sample.adc_t() << ", "
      << "tot: " << sample.tot() << ", ";
  else if (sample.isTOTComplete())
    s << "adc t-1: " << sample.adc_tm1() << ", "
      << "tot: " << sample.tot() << ", ";
  else
    s << "adc t-1: " << sample.adc_tm1() << ", "
      << "adc t: " << sample.adc_t() << ", ";

  s << "toa: " << sample.toa() << " }";
  return s;
}

std::ostream &operator<<(std::ostream &s,
                         const ldmx::HgcrocDigiCollection::HgcrocDigi &digi) {
  s << "HgcrocDigi { ";

  s << " Id: 0x" << std::hex << digi.id() << std::dec << " ";

  if (digi.isADC())
    s << "ADC Mode -> SOI: " << digi.soi() << " }";
  else
    s << "TOT Mode -> " << digi.tot() << " }";

  return s;
}

std::ostream &operator<<(std::ostream &s,
                         const ldmx::HgcrocDigiCollection &col) {
  s << "HgcrocDigiCollection { " << std::endl;
  for (unsigned int iDigi = 0; iDigi < col.getNumDigis(); iDigi++)
    s << "  " << col.getDigi(iDigi) << std::endl;
  s << "}";
  return s;
}
