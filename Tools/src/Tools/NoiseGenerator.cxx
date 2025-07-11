/**
 * @file NoiseGenerator.cxx
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/NoiseGenerator.h"

namespace ldmx {

NoiseGenerator::NoiseGenerator(double noiseValue, bool gauss) {
  noise_ = noiseValue;
  useGaussianModel_ = gauss;
  poisson_dist_ =
      std::make_unique<boost::math::poisson_distribution<> >(noiseValue);
}

void NoiseGenerator::seedGenerator(uint64_t seed) {
  random_ = std::make_unique<TRandom3>(seed);
}

std::vector<double> NoiseGenerator::generateNoiseHits(int emptyChannels) {
  if (random_.get() == nullptr) {
    EXCEPTION_RAISE("RandomSeedException",
                    "Noise generator was not seeded before use");
  }
  ldmx_log(trace) << "Empty channels: " << emptyChannels;
  ldmx_log(trace) << "Normalized integration limit: " << noiseThreshold_;

  double integral;
  if (useGaussianModel_)
    integral = ROOT::Math::normal_cdf_c(noiseThreshold_, noise_, pedestal_);
  else
    integral =
        boost::math::cdf(complement(*poisson_dist_, noiseThreshold_ - 1));
  ldmx_log(trace) << "Integral: " << integral;

  double noiseHitCount = random_->Binomial(emptyChannels, integral);
  ldmx_log(trace) << "# Noise hits: " << noiseHitCount;

  std::vector<double> noiseHits;
  for (int hitIndex = 0; hitIndex < noiseHitCount; ++hitIndex) {
    double rand = random_->Uniform();
    ldmx_log(trace) << "Rand: " << rand;
    double draw = integral * rand;
    ldmx_log(trace) << "Draw: " << draw;

    double cumulativeProb = 1.0 - integral + draw;
    ldmx_log(trace) << "Cumulative probability: " << cumulativeProb;

    double valueAboveThreshold;
    if (useGaussianModel_) {
      valueAboveThreshold =
          ROOT::Math::gaussian_quantile(cumulativeProb, noise_);
    } else {
      valueAboveThreshold =
          boost::math::quantile(*poisson_dist_, cumulativeProb);
    }
    ldmx_log(trace) << "Noise value: " << valueAboveThreshold;

    noiseHits.push_back(valueAboveThreshold);
  }

  return noiseHits;
}

}  // namespace ldmx
