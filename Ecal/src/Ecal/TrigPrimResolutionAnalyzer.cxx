#include "DetDescr/EcalID.h"
#include "DetDescr/EcalTriggerID.h"
#include "Ecal/EcalTriggerGeometry.h"
#include "Ecal/Event/EcalHit.h"
#include "Framework/EventProcessor.h"
#include "Recon/Event/HgcrocDigiCollection.h"
#include "Recon/Event/HgcrocTrigDigi.h"

namespace ldmx::ecal {
/**
 * Analyze the trigger primitives by comparing them to the precision hits
 *
 * This analyzer goes through and compares differents sums between the
 * trigger primitive level (groups of 9 cells for the ECal) and the
 * precision hit level (individual cells).
 */
class TrigPrimResolutionAnalyzer : public framework::Analyzer {
  std::string trig_collection_name_ = "ecalTrigDigis";
  std::string trig_pass_name_ = "";
  std::string hit_collection_name_ = "EcalRecHits";
  std::string hit_pass_name_ = "";
  double total_trigger_mean_ = 876.28120;
  double total_ampl_mean_ = 48.805244;
  double nominal_{1.0};
  double secondOrderEnergyCorrection_ = 4000. / 3940.5;
  double mip_si_energy_ = 0.13;
  std::vector<double> layerWeights_ = {
      2.312,  4.312,  6.522,  7.490,  8.595,  10.253, 10.915, 10.915, 10.915,
      10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915, 10.915,
      10.915, 10.915, 10.915, 10.915, 10.915, 14.783, 18.539, 18.539, 18.539,
      18.539, 18.539, 18.539, 18.539, 18.539, 18.539, 9.938};

 public:
  TrigPrimResolutionAnalyzer(const std::string& name,
                             framework::Process& process)
      : framework::Analyzer(name, process) {}
  virtual ~TrigPrimResolutionAnalyzer() = default;
  void configure(framework::config::Parameters& parameters) final;
  void onProcessStart() final;
  void analyze(const framework::Event& event) final;

  /**
   * Calculate the energy deposited estimate from a hit's amplitude
   *
   * We scale the amplitude by the layer weight with a few other factors.
   */
  double calculate_energy(int layer, double amplitude) {
    return (1 + layerWeights_[layer] / mip_si_energy_) * amplitude *
           secondOrderEnergyCorrection_;
  }
};

/**
 * macro to help avoid repetition on optional configuraiton parameters
 *
 * the default for a parameter is defined when it is declared in the class
 * declaration, here we provide its value to getParameter and so the user could
 * override the default if the python value is provided.
 *
 * Assuming the parameters class instance is called parameters.
 */
#define optional_update(variable) \
  variable##_ = parameters.getParameter(#variable, variable##_)

void TrigPrimResolutionAnalyzer::configure(
    framework::config::Parameters& parameters) {
  optional_update(trig_collection_name);
  optional_update(trig_pass_name);
  optional_update(hit_collection_name);
  optional_update(hit_pass_name);
  optional_update(total_trigger_mean);
  optional_update(total_ampl_mean);
  optional_update(secondOrderEnergyCorrection);
  optional_update(mip_si_energy);
  optional_update(layerWeights);

  // nominal scale separation between trigger and precision would be
  // determined experimentally by comparing their values
  // nominal_ = total_trigger_mean_ / total_ampl_mean_;
  // this has already been done so we just hardcode in the value
  // determined by Steven Metallo in 2024, but it can be updated
  // to use the ratio of the totals like above
  nominal_ = 18.14;
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
  std::vector<double> binsx_fin = {0.,  10., 20.,  30.,  40.,  50.,  60.,  70.,
                                   80., 90., 100., 150., 300., 575., 1000.};

  // initialize processing by making histograms and such
  // first, we get the directory for this processor in the histogram file
  getHistoDirectory();
  // then we can create histograms within it
  histograms_.create(
      "total_trig_energy" /* name - as written in output ROOT file */,
      "Total of all Trig Digis [MeV]" /* xlabel - axis label of histogram */,
      100 /* number of bins */, 0 /* minimum value */, 8000 /* maximum value */
  );
  histograms_.create("total_ampl_energy",
                     "Total of Precision Hit Amplitudes [MeV]", 100, 0, 8000);
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
                     100, 0, 6000, "Trigger / Full readout", 100, 0.95, 1.05);
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
 *
 * This is helpful for doing module sums where we want to sum over all trigger
 * or precision cells within a specific module. It acts like a key that can be
 * constructed from either a trigger cell ID or a precision cell ID.
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

/**
 * ordering operator required for UniqueModule to be used as a key in std::map
 */
bool operator<(const UniqueModule& lhs, const UniqueModule& rhs) {
  if (lhs.layer_ < rhs.layer_) return true;
  if (lhs.layer_ > rhs.layer_) return false;
  // lhs.layer_ == rhs.layer_
  return (lhs.module_ < rhs.module_);
}

/**
 * get the trigger primitive's estimate of the hit amplitude
 *
 * The trigger primitive digi class linearizes its packed estimate
 * for us, but there is one more shift on the low end.
 * Since the packing process drops the three lowest-order bits, we need
 * to shift the estimate up by 0.5 if the value is below 15.
 *
 * This also re-casts the integer into a float which is more readily comparable
 * to the float energy estimate stored within the reconstructed precision hits.
 */
float get_estimate(const HgcrocTrigDigi& trig) {
  uint32_t prim{trig.linearPrimitive()};
  if (prim < 15) {
    return (static_cast<float>(prim) + 0.5);
  }
  return static_cast<float>(prim);
}

void TrigPrimResolutionAnalyzer::analyze(const framework::Event& event) {
  // called once on each event, get objects and fill histograms
  const auto& trigs = event.getCollection<ldmx::HgcrocTrigDigi>(
      trig_collection_name_, trig_pass_name_);
  // trigs are a std::vector<ldmx::HgcrocTrigDigi>
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
    double trig_ampl = get_estimate(trig);
    double trig_energy = calculate_energy(mod.layer_, trig_ampl) / nominal_;

    /// add this trigger primitive to module sums
    if (module_sums.find(mod) == module_sums.end()) {
      module_sums[mod] = {0, 0.0};
    }

    module_sums[mod].first += trig_energy;
    trig_prim_total += trig_energy;
    if (mod.layer_ <= 20) {
      trig_prim_total_first20 += trig_energy;
    }

    // calculate the sum of precision hits for the 9 cells in this
    // trigger grouping
    double trig_group_prec_total{0};
    double trig_group_prec_total_unweight{0};
    for (auto& prec_id : geom.contentsOfTriggerCell(tid)) {
      for (const auto& hit : hits) {
        if (prec_id == hit.getID()) {
          trig_group_prec_total +=
              calculate_energy(mod.layer_, hit.getAmplitude());
          trig_group_prec_total_unweight += hit.getAmplitude();
        }
      }
    }

    // have trig_group_prec_total which is the sum of the precision hits
    // in that trigger group and trig_ampl is the sum
    // as reported by the trigger itself
    double ratio_energy = trig_energy / trig_group_prec_total,
           ratio_ampl = trig_ampl / trig_group_prec_total_unweight / nominal_;
    histograms_.fill("trig_group", trig_group_prec_total);
    histograms_.fill("trig_group_trigger", trig_energy * nominal_);
    histograms_.fill("trig_group_v_trig", trig_group_prec_total,
                     trig_energy * nominal_);
    histograms_.fill("trig_group_ampl_v_ampl", trig_group_prec_total,
                     ratio_energy);
    histograms_.fill("trig_group_ampl_v_ampl_varbin", trig_group_prec_total,
                     ratio_energy);
    histograms_.fill("trig_group_ampl_v_ampl_varbin_fin", trig_group_prec_total,
                     ratio_energy);
    histograms_.fill("trig_group_ampl_unweight", trig_group_prec_total_unweight,
                     ratio_ampl);
  }

  double prec_ampl_total{0.};
  double prec_ampl_total_first20{0.};
  for (const auto& hit : hits) {
    EcalID id{static_cast<unsigned int>(hit.getID())};
    UniqueModule mod{id};
    if (module_sums.find(mod) == module_sums.end()) {
      module_sums[mod] = {0, 0.0};
    }
    double prec_energy = calculate_energy(mod.layer_, hit.getAmplitude());
    module_sums[mod].second += prec_energy;
    prec_ampl_total += prec_energy;
    if (mod.layer_ <= 20) {
      prec_ampl_total_first20 += prec_energy;
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

  histograms_.fill("total_trig_energy", trig_prim_total);
  histograms_.fill("total_ampl_energy", prec_ampl_total);
  histograms_.fill("total_trig_v_total_ampl", trig_prim_total, prec_ampl_total);
  histograms_.fill("trig_ampl_nominal", trig_prim_total / prec_ampl_total);
  histograms_.fill("trig_ampl_v_ampl_total", prec_ampl_total,
                   trig_prim_total / prec_ampl_total);
  histograms_.fill("trig_ampl_v_ampl_total_first20", prec_ampl_total_first20,
                   trig_prim_total_first20 / prec_ampl_total_first20);
}

}  // namespace ldmx::ecal

DECLARE_ANALYZER(ldmx::ecal::TrigPrimResolutionAnalyzer);
