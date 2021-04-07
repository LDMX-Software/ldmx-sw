
#include "TFile.h"
#include "TTree.h"

#include "Framework/Configure/Parameters.h"
#include "Conditions/SimpleTableCondition.h"
#include "Tools/HgcrocEmulator.h"
#include "Recon/Event/HgcrocDigiCollection.h"

int main() {

  double gain = 320./20./1024;

  framework::config::Parameters parameters;
  parameters.addParameter("clockCycle", 25.);
  parameters.addParameter("timingJitter",0.25);
  parameters.addParameter("nADCs", 10 );
  parameters.addParameter("iSOI", 0 );
  parameters.addParameter("rateUpSlope",  -0.345);
  parameters.addParameter("timeUpSlope", 70.6547);
  parameters.addParameter("rateDnSlope", 0.140068);
  parameters.addParameter("timeDnSlope", 87.7649);
  parameters.addParameter("timePeak", 77.732);
  parameters.addParameter("noiseRMS", (700.+25.*20.)*(0.162/1000.)/20.);
  parameters.addParameter("noise",true);

  conditions::DoubleTableCondition chip_conditions("RUN_HGCROC_TABLE", {
    "PEDESTAL",
    "MEAS_TIME",
    "PAD_CAPACITANCE",
    "TOT_MAX",
    "DRAIN_RATE",
    "GAIN",
    "READOUT_THRESHOLD",
    "TOA_THRESHOLD",
    "TOT_THRESHOLD"
  });
  chip_conditions.setIdMask(0); // all ids are the same
  chip_conditions.add(0, {
    50. , //PEDESTAL 
    0.0, //MEAS_TIME - ns
    20., //PAD_CAPACITANCE - pF
    200., //TOT_MAX - ns - maximum time chip would be in TOT mode
    10240. / 200., //DRAIN_RATE - fC/ns
    gain, //GAIN - 320. fC / 1024. counts / 20 pF - conversion from ADC to mV
    50. + 3., //READOUT_THRESHOLD - 3 ADC counts above pedestal
    50.*gain + 5 *37*0.162/20., //TOA_THRESHOLD - mV - ~5  MIPs above pedestal
    50.*gain + 50*37*0.162/20., //TOT_THRESHOLD - mV - ~50 MIPs above pedestal
  });

  ldmx::HgcrocEmulator hgcroc(parameters);
  hgcroc.seedGenerator(420);
  hgcroc.condition(chip_conditions);

  double readout_threshold = gain*53.;
  double tot_threshold = 15.76225;
  
  float min_voltage_test{0.5};
  float max_voltage_test{20.};
  float voltage_step{0.1};
  int num_voltages{int((max_voltage_test-min_voltage_test)/voltage_step)};

  std::vector<float> time_tests {
    0., // nominal in-time with light speed particle
    0.2, // off-peak but within normal shower time
    1.0, // very late in shower
    20., // totally separate, probably cosmic
    -8.0, // pre-hit (e.g. cosmic)
  };

  // arbitrary cell id
  unsigned int cell_id = 0x14002020;

  TFile f("hgcroc_emulation.root","RECREATE");
  TTree t("hgcroc","hgcroc");
  
  float voltage, time;
  int adc{0}, tdc{0};
  bool readout, sim_totmode,
       digitized, digi_totmode;

  t.Branch("sim_voltage",&voltage);
  t.Branch("sim_time",&time);
  t.Branch("sim_readout",&readout);
  t.Branch("sim_totmode",&sim_totmode);

  t.Branch("digitized",&digitized);
  t.Branch("adc",&adc);
  t.Branch("digi_totmode",&digi_totmode);
  t.Branch("tdc",&tdc);

  ldmx::HgcrocDigiCollection all_digis;
  all_digis.setNumSamplesPerDigi(10);
  all_digis.setSampleOfInterestIndex(0);

  unsigned int last_digi=0;
  std::vector<std::pair<double,double>> the_pulse(1,{0.,0.});
  for (const float ti : time_tests) {
    for (int iv{0}; iv < num_voltages; iv++) {
      voltage = min_voltage_test + iv*voltage_step;
      time = ti;
      readout = (voltage > readout_threshold);
      sim_totmode = (voltage > tot_threshold);

      the_pulse[0].first  = voltage;
      the_pulse[0].second = time;

      std::vector<ldmx::HgcrocDigiCollection::Sample> digi_to_add;
      digitized = hgcroc.digitize(cell_id,the_pulse,digi_to_add);
      if (digitized) {
        all_digis.addDigi(cell_id, digi_to_add);

        auto digi = all_digis.getDigi(last_digi++);

        digi_totmode = digi.isTOT();
        if (digi_totmode) {
          adc = 0;
          tdc = digi.tot();
        } else {
          adc = digi.soi().adc_t();
          tdc = 0;
        }
      } else {
        adc = -1;
        tdc = -1;
      }
      t.Fill();
    }
  }

  t.Write();
  f.Close();
}
