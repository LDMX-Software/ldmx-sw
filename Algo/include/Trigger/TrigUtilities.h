#ifndef TRIGUTILITIES_H
#define TRIGUTILITIES_H
#include <vector>

namespace trigger {

    class ecalTpToE {
      public:
	/* double ecalTpToE(int tp, int layer){ */
	double calc(int tp, int layer){
	    float sie = hgc_compression_factor_ * tp *
		gain_ * mVtoMeV_;  // in MeV, before layer corrections
	    return (sie / mipSiEnergy_ * layerWeights_.at(layer) + sie) *
		secondOrderEnergyCorrection_;
	}
      private:
	float gain_ = 320. / 0.1 / 1024;                                    // mV/ADC
	float mVtoMeV_ = 0.130 / (37000.0 * (0.1602 / 1000.) * (1. / 0.1));  // MeV/mV
	std::vector<float> layerWeights_ = {
	    2.312, 4.312, 6.522, 7.490, 8.595, 10.253, 10.915, 10.915, 10.915, 10.915, 10.915,
	    10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915,
	    10.915, 10.915, 14.783, 18.539, 18.539, 18.539, 18.539, 18.539, 18.539, 18.539,
	    18.539, 18.539, 9.938};
	float secondOrderEnergyCorrection_ = 4000. / 3940.5;
	float mipSiEnergy_ = 0.130;
	int hgc_compression_factor_ = 8;
    };

}  // namespace trigger

#endif /* TRIGUTILITIES_H */
