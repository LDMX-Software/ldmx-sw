#include "Recon/Event/CompositePulse.h"

ClassImp(ldmx::CompositePulse);

namespace ldmx {
void CompositePulse::addOrMerge(const std::pair<double, double>& hit,
                                double hit_merge_ns) {
  auto imerge{hits_.begin()};
  for (; imerge != hits_.end(); imerge++)
    if (fabs(imerge->second - hit.second) < hit_merge_ns) break;
  if (imerge == hits_.end()) {  // didn't find a match, add to the list
    hits_.push_back(hit);
  } else {  // merge hits, shifting time to average
    imerge->second = (imerge->second * imerge->first + hit.first * hit.second);
    imerge->first += hit.first;
    imerge->second /= imerge->first;
  }
}

double CompositePulse::findCrossing(double low, double high, double level,
                                    double prec) {
  // use midpoint algorithm, assumes low is below and high is above
  double step = high - low;
  double pt = (high + low) / 2;
  while (step > prec) {
    double vmid = at(pt);
    if (vmid < level) {
      low = pt;
    } else {
      high = pt;
    }
    step = high - low;
    pt = (high + low) / 2;
  }
  return pt;
}

}  // namespace ldmx
