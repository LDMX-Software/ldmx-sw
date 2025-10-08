/**
 * @file NoiseGenerator.cxx
 * @brief Utility used to generate noise hits_.
 * @author Omar Moreno, SLAC National Accelerator Laboratory
 */

#include "Tools/NoiseGenerator.h"

namespace ldmx {

NoiseGenerator::NoiseGenerator(double noiseValue, bool gauss) {
  noise_ = noiseValue;
  use_gaussian_model_ = gauss;
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
  ldmx_log(trace) << "Normalized integration limit: " << noise_threshold_;

  double integral;
  if (use_gaussian_model_)
    integral = ROOT::Math::normal_cdf_c(noise_threshold_, noise_, pedestal_);
  else
    integral =
        boost::math::cdf(complement(*poisson_dist_, noise_threshold_ - 1));
  ldmx_log(trace) << "Integral: " << integral;

  double noise_hit_count = random_->Binomial(emptyChannels, integral);
  ldmx_log(trace) << "# Noise hits_: " << noise_hit_count;

  std::vector<double> noise_hits;
  for (int hit_index = 0; hit_index < noise_hit_count; ++hit_index) {
    double rand = random_->Uniform();
    ldmx_log(trace) << "Rand: " << rand;
    double draw = integral * rand;
    ldmx_log(trace) << "Draw: " << draw;

    double cumulative_prob = 1.0 - integral + draw;
    ldmx_log(trace) << "Cumulative probability: " << cumulative_prob;

    double value_above_threshold;
    if (use_gaussian_model_) {
      value_above_threshold =
          ROOT::Math::gaussian_quantile(cumulative_prob, noise_);
    } else {
      value_above_threshold =
          boost::math::quantile(*poisson_dist_, cumulative_prob);
    }
    ldmx_log(trace) << "Noise value: " << value_above_threshold;

    noise_hits.push_back(value_above_threshold);
  }

  return noise_hits;
}

}  // namespace ldmx
