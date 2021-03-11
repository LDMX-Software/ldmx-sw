/**
 * @file NoiseGenerator.cxx
 * @brief Utility used to generate noise hits.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/NoiseGenerator.h"
#include "Framework/Exception/Exception.h"

namespace ldmx {

NoiseGenerator::NoiseGenerator(double noiseValue, bool gauss) {
  noise_ = noiseValue;
  useGaussianModel_ = gauss;
  poisson_dist_ =
      std::make_unique<boost::math::poisson_distribution<> >(noiseValue);
}

NoiseGenerator::~NoiseGenerator() {}

void NoiseGenerator::seedGenerator(uint64_t seed) {
  random_ = std::make_unique<TRandom3>(seed);
}

std::vector<double> NoiseGenerator::generateNoiseHits(int emptyChannels) {
  if (random_.get() == nullptr) {
    EXCEPTION_RAISE("RandomSeedException",
                    "Noise generator was not seeded before use");
  }
  // std::cout << "[ Noise Generator ]: Empty channels: "
  //           << emptyChannels << std::endl;
  // std::cout << "[ Noise Generator ]: Normalized integration limit: "
  //          << noiseThreshold_ << std::endl;

  double integral;
  if (useGaussianModel_)
    integral = ROOT::Math::normal_cdf_c(noiseThreshold_, noise_, pedestal_);
  else
    integral =
        boost::math::cdf(complement(*poisson_dist_, noiseThreshold_ - 1));
  // std::cout << "[ Noise Generator ]: Integral: "
  //          << integral << std::endl;

  double noiseHitCount = random_->Binomial(emptyChannels, integral);
  // std::cout << "[ Noise Generator ]: # Noise hits: "
  //          << noiseHitCount << std::endl;

  std::vector<double> noiseHits;
  for (int hitIndex = 0; hitIndex < noiseHitCount; ++hitIndex) {
    double rand = random_->Uniform();
    // std::cout << "[ Noise Generator ]: Rand: "
    //          << rand << std::endl;
    double draw = integral * rand;
    // std::cout << "[ Noise Generator ]: Draw: "
    //          << draw << std::endl;

    double cumulativeProb = 1.0 - integral + draw;
    // std::cout << "[ Noise Generator ]: Cumulative probability: "
    //          << cumulativeProb << std::endl;

    double valueAboveThreshold;
    if (useGaussianModel_)
      valueAboveThreshold =
          ROOT::Math::gaussian_quantile(cumulativeProb, noise_);
    else
      valueAboveThreshold =
          boost::math::quantile(*poisson_dist_, cumulativeProb);
    // std::cout << "[ Noise Generator ]: Noise value: "
    //          << gaussAboveThreshold << std::endl;

    noiseHits.push_back(valueAboveThreshold);
  }

  return noiseHits;
}

}  // namespace ldmx
