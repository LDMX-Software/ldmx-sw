#include "TrigScint/AnalyticalRecHitProducer.h"

#include <iomanip>
#include <iostream>

#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"
#include "TLinearFitter.h"
#include "TMath.h"

namespace trigscint {

AnalyticalRecHitProducer::AnalyticalRecHitProducer(const std::string &name,
                                                   framework::Process &process)
    : Producer(name, process) {}

AnalyticalRecHitProducer::~AnalyticalRecHitProducer() {}

void AnalyticalRecHitProducer::configure(
    framework::config::Parameters &parameters) {
  // Configure this instance of the producer
  pedestal_ = parameters.getParameter<double>("pedestal");
  noise_ = parameters.getParameter<double>("elec_noise");
  gain_ = parameters.getParameter<double>("gain");
  mevPerMip_ = parameters.getParameter<double>("mev_per_mip");
  pePerMip_ = parameters.getParameter<double>("pe_per_mip");
  inputCollection_ = parameters.getParameter<std::string>("input_collection");
  inputPassName_ = parameters.getParameter<std::string>("input_pass_name");
  outputCollection_ = parameters.getParameter<std::string>("output_collection");
  verbose_ = parameters.getParameter<bool>("verbose");
  sample_of_interest_ = parameters.getParameter<int>("sample_of_interest");
  tdc_thr_ = parameters.getParameter<double>("tdc_thr");
  qie_sf_ = parameters.getParameter<double>("qie_sf");

  input_pulse_shape_ =
      parameters.getParameter<std::string>("input_pulse_shape");
  if (input_pulse_shape_ == "Expo") {
    pulse_params_.clear();
    pulse_params_.push_back(parameters.getParameter<double>("expo_k"));
    pulse_params_.push_back(parameters.getParameter<double>("expo_tmax"));

    ldmx_log(debug) << "expo_k =" << pulse_params_[0];
    ldmx_log(debug) << "expo_tmax =" << pulse_params_[1];
  }
}

void AnalyticalRecHitProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;

  // Ensure the sample of interest <4
  if (sample_of_interest_ > 3) {
    ldmx_log(error) << "sample_of_interest_ should be one of 0,1,2,3\n"
                    << "Currently, sample_of_interest = " << sample_of_interest_
                    << "\n";
    return;
  }

  // looper over sim hits and aggregate energy depositions
  // for each detID
  const auto digis{event.getCollection<trigscint::TrigScintQIEDigis>(
      inputCollection_, inputPassName_)};

  std::vector<ldmx::TrigScintHit> trigScintHits;
  for (const auto &digi : digis) {
    ldmx::TrigScintHit hit;
    auto adc{digi.getADC()};
    auto tdc{digi.getTDC()};

    hit.setModuleID(0);
    hit.setBarID(digi.getChanID());
    hit.setBeamEfrac(-1.);

    if (tdc[sample_of_interest_] > 49)
      hit.setTime(-999.);
    else
      hit.setTime(tdc[sample_of_interest_] * 0.5);

    auto Charge_time = ChargeReconstruction(adc, tdc, sample_of_interest_);
    auto Charge = Charge_time[0];
    auto time_est = Charge_time[1];

    hit.setAmplitude(Charge);
    hit.setEnergy(Charge * 6250. / gain_ * mevPerMip_ / pePerMip_);  // MeV
    hit.setPE(Charge * 6250. / gain_);
    if (time_est != -1) {hit.setTime(time_est);}
    if (hit.getTime() > 0.) {
      //std::cout << "adding hit " << digi.getChanID() << " " << time_est << " " <<  hit.getEnergy() << std::endl;
      trigScintHits.push_back(hit);
    }
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, trigScintHits);
}
std::vector<double>  AnalyticalRecHitProducer::ChargeReconstruction(std::vector<int> adc,
                                                        std::vector<int> tdc,
                                                        int sample) {
  int npulses = 0;               // No. of true pulses
  int poi = -1;                   // The pulse of interest
  std::vector<float> Charge_;    // stores pulse amplitudes
  std::vector<float> time_;    // stores pulse times
  auto Qdata = new Double_t[5];  // Linearized charge
  float tend = 1000 / qie_sf_;   // 1 time sample (in ns)
  float k_ = pulse_params_[0];
  float tmax_ = pulse_params_[1];
  float par0 = (exp(k_ * tmax_) - 1) / (k_ * tmax_) * exp(-k_ * tend);
  float alpha[2] = {par0,par0*(1-exp(-k_ * tend))};
  SimQIE qie;
  auto pulse = new Expo(k_, tmax_);

  for(int i=0;i<adc.size();i++){
    // Linearize charge, remove pedestal
    Qdata[i] = qie.ADC2Q(adc[i])-pedestal_;

    // Remove noise (2sigma noise cut)
    if(Qdata[i]<2*noise_)
      Qdata[i]=0;
  }
  
  for(int i=0;i<adc.size();i++){

    if(tdc[i]<40) {
      if (i == sample) poi = npulses;
      auto tm = tdc[i] / 2;  // measured time
      auto Qm = Qdata[i]-pulse->Integrate(tend * i, tend * (i + 1));  // remaining measured charge
      //float Qreco = (par0 * tdc_thr_ * tmax_ * exp(k_ * tm)-Qdata[i])/(par0 * exp(k_ * tm) - 1);
      float Qreco = (par0 * tdc_thr_ * tmax_ * exp(k_ * tm)-Qm)/(par0 * exp(k_ * tm) - 1);
      Charge_.push_back(Qreco);
      npulses++;
      pulse->AddPulse(tend * i + tdc[i] / 2, Qreco);
      time_.push_back(tdc[i] / 2);
    }
    else if (tdc[i] < 50) {
      if (i == sample)
	      poi = npulses;
      //Charge_.push_back(qie.ADC2Q(adc[i]));
      Charge_.push_back(Qdata[i]);
      //pulse->AddPulse(tend * i + tdc[i] / 2, qie.ADC2Q(adc[i]));
      pulse->AddPulse(tend * i + tdc[i] / 2, Qdata[i]);

      npulses++;
      time_.push_back(tdc[i] / 2);
    }
    else if(i<tdc.size()-1 && tdc[i+1]==62) {
      if (i == sample)
	poi = npulses;
      float Q0 = Qdata[i]-pulse->Integrate(tend*i, tend*(i+1));
      float Q1 = Qdata[i+1]-pulse->Integrate(tend*(i+1), tend*(i+2));
      float t0 = log((Q1)/(alpha[1]*Q0+alpha[0]*Q1))/k_;
      float Qreco = Q0/(1-alpha[0]*exp(k_*t0));      
      Charge_.push_back(Qreco);
      time_.push_back(t0);
      //std::cout << "tdc 62 " << t0 << " " <<  Qreco << std::endl;
      npulses++;
      pulse->AddPulse(tend * i + tdc[i] / 2, Qreco);
    }
  }

  /////////////// For Debigging purposes
  if (verbose_) {
    std::cout << outputCollection_ << std::endl;
    std::cout << "TS \t|\t0\t|\t1\t|\t2\t|\t3\t|\t4\t|\n"
              << "---------------------------------------------"
              << "--------------------------------------------\n"
              << "tdc \t|";
    for (int i = 0; i < 5; i++) std::cout << std::setw(10) << tdc[i] << "\t|";

    std::cout << "\nadc \t|";
    for (int i = 0; i < 5; i++) std::cout << std::setw(10) << adc[i] << "\t|";

    std::cout << "\nQdata\t|";
    for (int i = 0; i < 5; i++) std::cout << std::setw(10) << Qdata[i] << "\t|";

    std::cout << "\n---------------------------------------------"
              << "--------------------------------------------";
    for (int n = 0; n < npulses; n++) {
      std::cout << std::setw(10) << "\nPulse" << n << "\t|";
      for (int i = 0; i < 5; i++)
        std::cout << std::setw(10) << pulse->Integrate(i * tend, (i + 1) * tend)
                  << "\t|";
    }

    std::cout << "\n"
              << "\nnpulses = " << npulses << std::endl
              << "poi = " << poi << std::endl;

    std::cout << "Charge_[]: ";
    for (int i = 0; i < Charge_.size(); i++)
      std::cout << std::setw(10) << Charge_[i] << "\t|";
    std::cout << "\n";
  }
  std::vector<double> poi_vals;
  if(poi == -1) {
    poi_vals = {-1,-1};
  }
  else {poi_vals = {Charge_[poi], time_[poi]};}
  return poi_vals; //Charge_[poi];
}
}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, AnalyticalRecHitProducer);