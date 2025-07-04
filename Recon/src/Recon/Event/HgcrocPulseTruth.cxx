#include "Recon/Event/HgcrocPulseTruth.h"

ClassImp(ldmx::HgcrocPulseTruth);

    namespace ldmx {
  void HgcrocPulseTruth::Clear() {}

  // Could be improved...
  double HgcrocPulseTruth::getMax() const {
    auto hits = compositePulse_.hits();

    std::sort(
        hits.begin(), hits.end(),
        [](const std::pair<double, double> &a,
           const std::pair<double, double> &b) { return a.second < b.second; });
    double starttime = hits.at(0).second - 100;
    double endtime = hits.at(hits.size() - 1).second + 200;

    double peak = -9999.0;
    for (int i = 0; i < 100; i++) {
      double time = starttime + (endtime - starttime) / 100.0 * i;
      double v = compositePulse_.at(time);
      if (v > peak) peak = v;
    }

    return peak;
  }

}  // namespace ldmx
