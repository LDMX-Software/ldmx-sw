#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace ldmx::ecal {
class TrigPrimResolutionAnalyzer : public framework::Analyzer {
  std::string digi_collection_name_ = "EcalDigis";
  std::string digi_pass_name_ = "";
  std::string trig_collection_name_ = "ecalTrigDigis";
  std::string trig_pass_name_ = "";
  std::string hit_collection_name_ = "EcalRecHits";
  std::string hit_pass_name_ = "";

 public:
  TrigPrimResolutionAnalyzer(const std::string& name,
                             framework::Process& process)
      : framework::Analyzer(name, process) {}
  virtual ~TrigPrimResolutionAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) final;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;
};

void TrigPrimResolutionAnalyzer::configure(
    framework::config::Parameters& parameters) {
  /*
   * Since I defined these member variables to have sensible values in the
   * declaration above, we DO NOT get their parameter values from the python
   * config. Below, I have written out the lines that we could use to get their
   * values from the python config if changing them is desirable (e.g. if you
   * want to run a few different passes of digi and recon with different
   * parameters and then want to analyze the different passes).
   */
  /*
  digi_collection_name_ = parameters.getParameter("digi_collection_name");
  digi_pass_name_ = parameters.getParameter("digi_pass_name");
  trig_collection_name_ = parameters.getParameter("trig_collection_name");
  trig_pass_name_ = parameters.getParameter("trig_pass_name");
  hit_collection_name_ = parameters.getParameter("hit_collection_name");
  hit_pass_name_ = parameters.getParameter("hit_pass_name");
  */
  //  digiCollName_ = parameters.getParameter<std::string>("digiCollName");
  //  digiPassName = parameters.getParameter<std::string>("digiPassName");
  //  condObjName_ =
  //	  parameters.getParameter<std::string>("condObjName","EcalTrigPrimDigiConditions");
}

void TrigPrimResolutionAnalyzer::onProcessStart() {
  // variable bins
  std::vector<double> binsx = {0.,   10.,  20.,  30.,  40.,  50.,  60.,
                               70.,  80.,  90.,  100., 110., 120., 130.,
                               140., 150., 300., 575., 1000.};
  std::vector<double> binsy = {
      0.6,  0.608, 0.616, 0.624, 0.632, 0.64, 0.648, 0.656, 0.664, 0.672,
      0.68, 0.688, 0.696, 0.704, 0.712, 0.72, 0.728, 0.736, 0.744, 0.752,
      0.76, 0.768, 0.776, 0.784, 0.792, 0.8,  0.808, 0.816, 0.824, 0.832,
      0.84, 0.848, 0.856, 0.864, 0.872, 0.88, 0.888, 0.896, 0.904, 0.912,
      0.92, 0.928, 0.936, 0.944, 0.952, 0.96, 0.968, 0.976, 0.984, 0.992,
      1.,   1.008, 1.016, 1.024, 1.032, 1.04, 1.048, 1.056, 1.064, 1.072,
      1.08, 1.088, 1.096, 1.104, 1.112, 1.12, 1.128, 1.136, 1.144, 1.152,
      1.16, 1.168, 1.176, 1.184, 1.192, 1.2,  1.208, 1.216, 1.224, 1.232,
      1.24, 1.248, 1.256, 1.264, 1.272, 1.28, 1.288, 1.296, 1.304, 1.312,
      1.32, 1.328, 1.336, 1.344, 1.352, 1.36, 1.368, 1.376, 1.384, 1.392,
      1.4};

  std::vector<double> binsx_fin = {0., 10., 20., 30., 40., 50., 60., 70.,
	                           80., 90., 100., 150., 300., 575., 1000.};

  // initialize processing by making histograms and such
  // first, we get the directory for this processor in the histogram file
  getHistoDirectory();
  // then we can create histograms within it
  histograms_.create(
      "total_trig_energy" /* name - as written in output ROOT file */,
      "Total of all Trig Digis [MeV]" /* xlabel - axis label of histogram */,
      100 /* number of bins */, 0 /* minimum value */, 8000 /* maximum value */
  );  // WARNING: I don't think this binning is good!! I just picked a random
      // number!!
  histograms_.create("total_ampl_energy",
                     "Total of Precision Hit Amplitudes [MeV]", 100, 0, 8000);
  /*
   * 2D Histograms are also possible
  histograms_.create(
      "name",
      "xlabel", nbins, xstart, xend,
      "ylabel", nbins, ystart, yend
  );
  */
  histograms_.create("total_trig_v_total_ampl", "Total of all Trig Digis [MeV]",
                     1000, 0, 8000, "Total of Precision Hit Amplitudes [MeV]",
                     100, 0, 8000);
  histograms_.create("trig_ampl_nominal", "Total trig / total ampl", 100, 0.85,
                     1.15);
  histograms_.create("module_id", "Module id", 7, 0, 7);
  histograms_.create("layer_id", "Layer id", 34, 0, 34);
  histograms_.create("trig_sum_per_module", "module trigger sum [MeV]", 100, 0,
                     2000);
  histograms_.create("ampl_sum_per_module", "module precision ampl sum [MeV]",
                     100, 0, 2000);
  histograms_.create("trig_v_ampl_module", "Module-sum full readout [MeV]", 100,
                     0, 2000, "Module-sum trigger [MeV]", 100, 0, 2000);
  histograms_.create("trig_sum_per_layer", "layer trigger sum [MeV]", 100, 0,
                     2000);
  histograms_.create("ampl_sum_per_layer", "layer precision ampl sum [MeV]",
                     100, 0, 2000);
  histograms_.create("trig_v_ampl_layer", "Layer trigger total [MeV]", 100, 0,
                     2500, "Layer ampl total [MeV]", 100, 0, 2500);
  histograms_.create("trig_ampl_v_ampl", "module precision ampl sum [MeV]", 100,
                     0, 1000, "module trigger / module precision ampl", 100,
                     0.4, 1.2);
  histograms_.create("trig_ampl_v_ampl_binadjust",
                     "module precision trig sum [MeV]", 100, 0, 2000,
                     "module trigger / module precision ampl", 100, 0.4, 1.2);
  histograms_.create("trig_ampl_v_ampl_layer", "layer precision ampl sum [MeV]",
                     100, 0, 1000, "layer trigger / layer precision ampl", 100,
                     0.4, 1.2);
  histograms_.create("trig_ampl_v_ampl_total", "total precision ampl sum [MeV]",
                     100, 0, 8000, "total trigger / total precision ampl", 100,
                     0.9, 1.1);
  histograms_.create("trig_ampl_v_ampl_total_first20", "Full readout sum [MeV]",
                     50, 0, 6000, "Trigger / Full readout", 100, 0.95, 1.05);
  histograms_.create("trig_group", "trigger group total precision hits [MeV]",
                     100, 0, 2000);
  histograms_.create("trig_group_trigger", "trigger group trigger [MeV]", 100,
                     0, 8000);
  histograms_.create("trig_group_v_trig",
                     "trigger group total precision hits [MeV]", 100, 0, 2000,
                     "trigger group trigger [MeV]", 100, 0, 8000);
  histograms_.create("trig_group_ampl_v_ampl",
                     "Trigger group full readout [MeV]", 100, 0, 1000,
                     "Trigger group ratio / nominal", 200, 0.6, 1.4);
  histograms_.create("trig_group_ampl_v_ampl_varbin",
                     "Trigger group full readout [MeV]", binsx,
                     "Trigger group ratio / nominal", binsy);
  histograms_.create("trig_group_ampl_unweight",
                     "unweighted trigger group total precision hits [MeV]", 100,
                     0, 20,
                     "trigger group trigger / unweighted trigger group total "
                     "prec hits / nominal",
                     200, 0.6, 1.4);
  histograms_.create("trig_group_ampl_v_ampl_varbin_fin",
                     "Trigger group full readout [MeV]", binsx_fin,
                     "Trigger group ratio / nominal", binsy);
}

/**
 * structure holding data uniquely identifying a specific module in the ECal
 */
struct UniqueModule {
  unsigned int layer_;
  unsigned int module_;
  UniqueModule(EcalTriggerID tid) {
    layer_ = tid.getLayerID();
    module_ = tid.getModuleID();
  }
  UniqueModule(EcalID eid) {
    layer_ = eid.getLayerID();
    module_ = eid.getModuleID();
  }
};

// get the trigger primitive's estimate of the hit amplitude
float get_estimate(const HgcrocTrigDigi& trig) {
  uint32_t prim{trig.linearPrimitive()};
  if (prim < 15) {
    return (static_cast<float>(prim) + 0.5);
  }
  return static_cast<float>(prim);
}

const double total_trigger_mean = 876.28120;
const double total_ampl_mean = 48.805244;
// const double nominal = total_trigger_mean / total_ampl_mean;
// nominal adjust for presentation
const double nominal = 18.14;
const double secondOrderEnergyCorrection = 4000. / 3940.5;
const double mip_si_energy = 0.13;
double layerWeights[34] = {
    2.312,  4.312,  6.522,  7.490,  8.595,  10.253, 10.915, 10.915, 10.915,
    10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915,
    10.915, 10.915, 10.915, 10.915, 10.915, 14.783, 18.539, 18.539, 18.539,
    18.539, 18.539, 18.539, 18.539, 18.539, 18.539, 9.938};

/**
 * ordering operator required for UniqueModule to be used as a key in std::map
 */
bool operator<(const UniqueModule& lhs, const UniqueModule& rhs) {
  if (lhs.layer_ < rhs.layer_) return true;
  if (lhs.layer_ > rhs.layer_) return false;
  // lhs.layer_ == rhs.layer_
  return (lhs.module_ < rhs.module_);
}

void TrigPrimResolutionAnalyzer::analyze(const framework::Event& event) {
  // called once on each event, get objects and fill histograms
  const auto& trigs = event.getCollection<ldmx::HgcrocTrigDigi>(
      trig_collection_name_, trig_pass_name_);
  // trigs are a std::vector<ldmx::HgcrocTrigDigi>
  const auto& digis = event.getObject<ldmx::HgcrocDigiCollection>(
      digi_collection_name_, digi_pass_name_);
  // digis are a ldmx::HgcrocDigiCollection
  const auto& hits =
      event.getCollection<ldmx::EcalHit>(hit_collection_name_, hit_pass_name_);
  // hits are a std::vector<ldmx::EcalHit>
  const ::ecal::EcalTriggerGeometry& geom =
      getCondition<::ecal::EcalTriggerGeometry>(
          ::ecal::EcalTriggerGeometry::CONDITIONS_OBJECT_NAME);

  std::map<UniqueModule, std::pair<int, double>> module_sums;
  int trig_prim_total{0};
  int trig_prim_total_first20{0};
  for (const auto& trig : trigs) {
    EcalTriggerID tid{trig.getId()};
    UniqueModule mod{tid};
    if (module_sums.find(mod) == module_sums.end()) {
      module_sums[mod] = {0, 0.0};
    }
    module_sums[mod].first += (1. + layerWeights[mod.layer_] / mip_si_energy) *
                              secondOrderEnergyCorrection / nominal *
                              get_estimate(trig);
    trig_prim_total += (1. + layerWeights[mod.layer_] / mip_si_energy) *
                       secondOrderEnergyCorrection / nominal *
                       get_estimate(trig);
    if (mod.layer_ <= 20) {
      trig_prim_total_first20 +=
          (1. + layerWeights[mod.layer_] / mip_si_energy) *
          secondOrderEnergyCorrection / nominal * get_estimate(trig);
    }
  }

  double prec_ampl_total{0.};
  double prec_ampl_total_first20{0.};
  for (const auto& hit : hits) {
    EcalID id{hit.getID()};
    UniqueModule mod{id};
    if (module_sums.find(mod) == module_sums.end()) {
      module_sums[mod] = {0, 0.0};
    }
    module_sums[mod].second += (1. + layerWeights[mod.layer_] / mip_si_energy) *
                               secondOrderEnergyCorrection * hit.getAmplitude();
    prec_ampl_total += (1. + layerWeights[mod.layer_] / mip_si_energy) *
                       secondOrderEnergyCorrection * hit.getAmplitude();
    if (mod.layer_ <= 20) {
      prec_ampl_total_first20 +=
          (1. + layerWeights[mod.layer_] / mip_si_energy) *
          secondOrderEnergyCorrection * hit.getAmplitude();
    }
  }

  std::map<int, std::pair<int, double>> layer_sums;
  for (const auto& [unique_module, sum_pair] : module_sums) {
    if (layer_sums.find(unique_module.layer_) == layer_sums.end()) {
      // layer is not found in layer_sums map so start the sum at 0
      layer_sums[unique_module.layer_] = {0, 0.0};
    }
    // add this module total to running total for the layer
    layer_sums[unique_module.layer_].first += sum_pair.first;
    layer_sums[unique_module.layer_].second += sum_pair.second;
  }

  for (const auto& trig : trigs) {
    EcalTriggerID tid{trig.getId()};
    UniqueModule mod{tid};
    double trig_group_prec_total{0};
    double trig_group_prec_total_unweight{0};
    for (auto& prec_id : geom.contentsOfTriggerCell(tid)) {
      for (const auto& hit : hits) {
        if (prec_id == hit.getID()) {
          trig_group_prec_total +=
              (1. + layerWeights[mod.layer_] / mip_si_energy) *
              secondOrderEnergyCorrection * hit.getAmplitude();
          trig_group_prec_total_unweight += hit.getAmplitude();
        }
      }
    }

    // have trig_group_total which is the sum of the precision hits
    // in that trigger group and get_estimate(trig) is the sum
    // as reported by the trigger itself
    histograms_.fill("trig_group", trig_group_prec_total);
    histograms_.fill(
        "trig_group_trigger",
        (1. + layerWeights[mod.layer_] / mip_si_energy) *
            secondOrderEnergyCorrection *
            get_estimate(trig));  // get_estiamte instead of get_estimate(trig)
    histograms_.fill("trig_group_v_trig", trig_group_prec_total,
                     (1. + layerWeights[mod.layer_] / mip_si_energy) *
                         secondOrderEnergyCorrection * get_estimate(trig));
    histograms_.fill("trig_group_ampl_v_ampl", trig_group_prec_total,
                     (1. + layerWeights[mod.layer_] / mip_si_energy) *
                         secondOrderEnergyCorrection * get_estimate(trig) /
                         trig_group_prec_total / nominal);
    histograms_.fill("trig_group_ampl_v_ampl_varbin", trig_group_prec_total,
                     (1. + layerWeights[mod.layer_] / mip_si_energy) *
                         secondOrderEnergyCorrection * get_estimate(trig) /
                         trig_group_prec_total / nominal);
    histograms_.fill("trig_group_ampl_v_ampl_varbin_fin", trig_group_prec_total,
                     (1. + layerWeights[mod.layer_] / mip_si_energy) *
                         secondOrderEnergyCorrection * get_estimate(trig) /
                         trig_group_prec_total / nominal);
    histograms_.fill(
        "trig_group_ampl_unweight", trig_group_prec_total_unweight,
        get_estimate(trig) / trig_group_prec_total_unweight / nominal);
  }

  int layer_id_tmp = {0};
  int module_id_tmp = {0};
  for (const auto& [mod_id, sum_pair] : module_sums) {
    /*    std::cout << " layer id " << mod_id.layer_ << " mod id " <<
     * mod_id.module_ << std::endl; */
    layer_id_tmp = mod_id.layer_;
    module_id_tmp = mod_id.module_;
    histograms_.fill("layer_id", layer_id_tmp);
    histograms_.fill("module_id", module_id_tmp);
    histograms_.fill("trig_sum_per_module", sum_pair.first);
    histograms_.fill("ampl_sum_per_module", sum_pair.second);
    histograms_.fill("trig_v_ampl_module", sum_pair.second, sum_pair.first);
    histograms_.fill("trig_ampl_v_ampl", sum_pair.second,
                     sum_pair.first / sum_pair.second);
    histograms_.fill("trig_ampl_v_ampl_binadjust", sum_pair.second,
                     sum_pair.first / sum_pair.second);
  }

  for (const auto& [layer, layer_sum_pair] : layer_sums) {
    histograms_.fill("trig_sum_per_layer", layer_sum_pair.first);
    histograms_.fill("ampl_sum_per_layer", layer_sum_pair.second);
    histograms_.fill("trig_v_ampl_layer", layer_sum_pair.first,
                     layer_sum_pair.second);
    histograms_.fill("trig_ampl_v_ampl_layer", layer_sum_pair.second,
                     layer_sum_pair.first / layer_sum_pair.second);
  }

  /*
   * after picking some dummy bins, I printout the values I'm going to fill
   * so I can see what an actual binning should be. For 10 events, the output
  was
   * 978
   * 949
   * 1070
   * 858
   * 846
   * 774
   * 853
   * 859
   * 887
   * 917
   * so I changed the binning to 1k bins ranging from 0 to 10k
  std::cout << trig_prim_total << std::endl;
   */
  histograms_.fill("total_trig_energy", trig_prim_total);
  histograms_.fill("total_ampl_energy", prec_ampl_total);
  histograms_.fill("total_trig_v_total_ampl", trig_prim_total, prec_ampl_total);
  histograms_.fill("trig_ampl_nominal", trig_prim_total / prec_ampl_total);
  histograms_.fill("trig_ampl_v_ampl_total", prec_ampl_total,
                   trig_prim_total / prec_ampl_total);
  histograms_.fill("trig_ampl_v_ampl_total_first20", prec_ampl_total_first20,
                   trig_prim_total_first20 / prec_ampl_total_first20);

  /*  for (const auto& [ unique_module, sum_pair ] : module_sums) {
      std::cout << unique_module.layer_ << " " << unique_module.module_
        << " -> " << sum_pair.first << " " << sum_pair.second << std::endl;
    }
    */

  /*
   * 2D fill example
  histograms_.fill("name", xvalue, yvalue);
  */
}

}  // namespace ldmx::ecal

DECLARE_ANALYZER_NS(ldmx::ecal, TrigPrimResolutionAnalyzer);
