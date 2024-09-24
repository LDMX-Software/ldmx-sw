#include "TrigScint/NumericalRecHitProducer.h"
#include "Framework/Exception/Exception.h"
#include "Framework/RandomNumberSeedService.h"
#include "TMath.h"

#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

#include <iostream>
#include "TLinearFitter.h"
#include<iomanip>

namespace trigscint {

  float tm_new=0;
  std::vector<float> Qm_new;
  float TimePeriod=0;		// time periodd of 1 TS
  float k_,tmax_;
  int npulses=0;
  double NewCostFunction(const double* params);
  
NumericalRecHitProducer::NumericalRecHitProducer(const std::string &name,
                                                 framework::Process &process)
    : Producer(name, process) {}

NumericalRecHitProducer::~NumericalRecHitProducer() {}

void NumericalRecHitProducer::configure(
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

    k_ = pulse_params_[0];
    tmax_ = pulse_params_[1];

    ldmx_log(debug) << "expo_k =" << pulse_params_[0];
    ldmx_log(debug) << "expo_tmax =" << pulse_params_[1];
  }

}

  void NumericalRecHitProducer::produce(framework::Event &event) {
  // initialize QIE object for linearizing ADCs
  SimQIE qie;

  // Ensure the sample of interest <4
  if(sample_of_interest_>3) {
    ldmx_log(error)<<"sample_of_interest_ should be one of 0,1,2,3\n"
		   <<"Currently, sample_of_interest = "<<sample_of_interest_
		   <<"\n";
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

    auto Charge =
      ChargeReconstruction(adc,tdc,sample_of_interest_);

    hit.setAmplitude(Charge);
    hit.setEnergy(Charge * 6250. /
		  gain_ * mevPerMip_ / pePerMip_);  // MeV
    hit.setPE(Charge * 6250. /gain_);
    trigScintHits.push_back(hit);
  }
  // Create the container to hold the
  // digitized trigger scintillator hits.

  event.add(outputCollection_, trigScintHits);
}
  Double_t NumericalRecHitProducer::ChargeReconstruction
  (std::vector<int>adc,std::vector<int>tdc,int sample) {
    
    npulses = 0;                  // No. of true pulses
    int poi=0;                    // The pulse of interest
    std::vector<float> Charge_;	  // stores pulse amplitudes
    std::vector<double> InitVal;  // Initial values of fitting parameters
    auto Qdata = new Double_t[5]; // Linearized charge
    float tend = 1000/qie_sf_;	  // 1 time sample (in ns)
    TimePeriod = tend;
    SimQIE qie;
    auto pulse = new Expo(k_,tmax_);

    // Initialize the minimizer.
    ROOT::Math::Minimizer* min = 
      ROOT::Math::Factory::CreateMinimizer("Minuit","Migrad");
    
    
    // if(tdc[sample]==63) return(0); // no signal pulse

    Qm_new.clear();
    for(int i=0;i<tdc.size();i++) {
      Qdata[i] = qie.ADC2Q(adc[i]);

      // Clean up the measured charge
      if(Qdata[i]<pedestal_+2*noise_)
    	Qm_new.push_back(0);
      else
    	Qm_new.push_back(Qdata[i]-pedestal_);
      
      // Find the pulses to fit
      if(tdc[i]<50 ||
    	 (i<tdc.size()-1 && tdc[i+1]==62)) {
      	if(i==sample_of_interest_)
      	  poi=npulses;
    	if(tdc[i]<50)
    	  InitVal.push_back(tend*i+tdc[i]/2);
    	else
    	  InitVal.push_back(tend*i);
    	InitVal.push_back(Qm_new[i]);
    	npulses++;
      }
    }

    if(npulses==0) {
      if(verbose_) {
	ldmx_log(error)<<"No pulse to fit. npulses = 0\n";
	ldmx_log(error)<<"tdc "
		       <<std::setw(10)<<tdc[0]
		       <<std::setw(10)<<tdc[1]
		       <<std::setw(10)<<tdc[2]
		       <<std::setw(10)<<tdc[3]
		       <<std::setw(10)<<tdc[4];
	ldmx_log(error)<<"adc "
		       <<std::setw(10)<<adc[0]
		       <<std::setw(10)<<adc[1]
		       <<std::setw(10)<<adc[2]
		       <<std::setw(10)<<adc[3]
		       <<std::setw(10)<<adc[4];
	ldmx_log(error)<<"Qm_new "
		       <<std::setw(10)<<Qm_new[0]
		       <<std::setw(10)<<Qm_new[1]
		       <<std::setw(10)<<Qm_new[2]
		       <<std::setw(10)<<Qm_new[3]
		       <<std::setw(10)<<Qm_new[4];
	ldmx_log(error)<<"returning linear(adc["<<sample
		       <<"]+adc["<<sample+1<<"])";
      }
      return(Qm_new[sample]+Qm_new[sample+1]);
    }
    
    ROOT::Math::Functor f(&NewCostFunction,2*npulses);
    min->SetFunction(f);

    for(int n = 0;n<npulses;n++) {
      char* id = new char[5];
      sprintf(id,"t%d",n+1);
      min->SetVariable(2*n,id,InitVal[2*n],0.1);
      sprintf(id,"Q%d",n+1);
      min->SetVariable(2*n+1,id,InitVal[2*n+1],1);
    }
    
    min->SetMaxFunctionCalls(1000000); // for Minuit/Minuit2 
    min->SetMaxIterations(10000);  // for GSL
    min->SetTolerance(0.001);
    // if(verbose_)
    //   min->SetPrintLevel(1);

    min->Minimize();
    const double *pred = min->X();

    // pulse->AddPulse(tend*i+pred[0],pred[1]);
    // }

    /////////////// For Debigging purposes
    // if(verbose_ || pred[2*poi+1]<0) {
    // if(verbose_ || pred[2*poi+1]*6250./gain_ * mevPerMip_ / pePerMip_> 1.6) {
    if(verbose_){
      std::cout<<"TS \t|\t0\t|\t1\t|\t2\t|\t3\t|\t4\t|\n"
	       <<"---------------------------------------------"
	       <<"--------------------------------------------\n"
	       <<"tdc \t|";
      for(int i=0;i<5;i++)
	std::cout<<std::setw(10)<<tdc[i]<<"\t|";
    
      std::cout<<"\nadc \t|";
      for(int i=0;i<5;i++)
	std::cout<<std::setw(10)<<adc[i]<<"\t|";
    
      std::cout<<"\nQdata\t|";
      for(int i=0;i<5;i++)
	std::cout<<std::setw(10)<<Qdata[i]<<"\t|";

      std::cout<<"\nQm_new\t|";
      for(int i=0;i<5;i++)
	std::cout<<std::setw(10)<<Qm_new[i]<<"\t|";

      std::cout<<"\n---------------------------------------------"
	       <<"--------------------------------------------";

      for(int n = 0;n<npulses;n++){
      	std::cout<<std::setw(10)<<"\nPulse"<<n<<"\t|";
      	auto pls = new Expo(k_,tmax_);
      	pls->AddPulse(pred[2*n],pred[2*n+1]);
      	for(int i=0;i<5;i++)
      	  std::cout<<std::setw(10)<<pls->Integrate(i*tend,(i+1)*tend)<<"\t|";
      	std::cout<<" Q= "<<pred[2*n+1];
      	std::cout<<" t0= "<<pred[2*n];
      }
      std::cout<<"\n"
      	       <<"\nnpulses = "<<npulses<<std::endl
      	       <<"poi = "<<poi<<std::endl;
      std::cout<<"\n\n";
    }

    return pred[2*poi+1];
  }

  double NewCostFunction(const double* params) {
    double cost=0;
    auto pulse = new Expo(k_,tmax_);
    for(int n=0;n<npulses;n++)
      pulse->AddPulse(params[2*n],params[2*n+1]);

    // std::cout<<"\nQpred\t|";
    for(int i=0;i<5;i++){
      double Qpred = pulse->Integrate(i*TimePeriod,(i+1)*TimePeriod);
      cost += pow(Qm_new[i]-Qpred,2);
      // std::cout<<std::setw(10)<<Qpred<<"\t|";
    }

    // std::cout<<"params: "<<std::setw(10)<<params[0]<<"\t|"<<std::setw(10)<<params[1]<<"\t|";
    // std::cout<<"cost: "<<std::setw(10)<<cost<<"\n";
    return cost;
  }

  double NumericalRecHitProducer::CostFunction(const double* params) {
    SimQIE qie;
    auto pulse = new Expo(pulse_params_[0],pulse_params_[1]);
    pulse->AddPulse(params[0],params[1]);
    pulse->AddPulse(params[2],params[3]);

    double Qpred = pulse->Integrate(0,1000/qie_sf_);
    double Tpred = qie.TDC(pulse,0)/2;
    
    double cost1 = pow(tm-Tpred,2);
    double cost2 = pow(Qm-Qpred,2);
    return(cost1+cost2);
  }
}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, NumericalRecHitProducer);