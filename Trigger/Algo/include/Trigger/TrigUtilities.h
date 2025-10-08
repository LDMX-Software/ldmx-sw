#ifndef TRIGUTILITIES_H
#define TRIGUTILITIES_H
#include <vector>

namespace trigger {

class EcalTpToE {
 public:
  /* double ecalTpToE(int tp, int layer){ */
  double calc(int tp, int layer) {
    float sie = hgc_compression_factor_ * tp * gain_ *
                m_vto_me_v_;  // in MeV, before layer corrections
    return (sie / mip_si_energy_ * layer_weights_.at(layer) + sie) *
           second_order_energy_correction_;
  }

 private:
  float gain_ = 320. / 0.1 / 1024;  // mV/ADC
  float m_vto_me_v_ =
      0.130 / (37000.0 * (0.1602 / 1000.) * (1. / 0.1));  // MeV/mV
  std::vector<float> layer_weights_ = {
      2.312,  4.312,  6.522,  7.490,  8.595,  10.253, 10.915, 10.915, 10.915,
      10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915,
      10.915, 10.915, 10.915, 10.915, 10.915, 14.783, 18.539, 18.539, 18.539,
      18.539, 18.539, 18.539, 18.539, 18.539, 18.539, 9.938};
  float second_order_energy_correction_ = 4000. / 3940.5;
  float mip_si_energy_ = 0.130;
  int hgc_compression_factor_ = 8;
};

}  // namespace trigger

#endif /* TRIGUTILITIES_H */
