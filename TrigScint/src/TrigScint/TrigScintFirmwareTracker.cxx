
#include "TrigScint/TrigScintFirmwareTracker.h"
#include "TrigScint/trackproducer.h"
#include "TrigScint/clusterproducer.h"
#include "TrigScint/objdef.h"
#include <iterator>  // std::next
#include <map>

namespace trigscint {

void TrigScintFirmwareTracker::configure(framework::config::Parameters &ps) {
  minThr_ = ps.getParameter<double>("clustering_threshold");
  digis1_collection_ = ps.getParameter<std::string>("digis1_collection");
  digis2_collection_ = ps.getParameter<std::string>("digis2_collection");
  digis3_collection_ = ps.getParameter<std::string>("digis3_collection");
  passName_ = ps.getParameter<std::string>("input_pass_name");
  output_collection_ = ps.getParameter<std::string>("output_collection");
  verbose_ = ps.getParameter<int>("verbosity");
  timeTolerance_ = ps.getParameter<double>("time_tolerance");
  padTime_ = ps.getParameter<double>("pad_time");
  if (verbose_) {
    ldmx_log(info) << "In TrigScintFirmwareTracker: configure done!";
    ldmx_log(info) << "\nClustering threshold: " << minThr_
                   << "\nExpected pad hit time: " << padTime_
                   << "\nMax hit time delay: " << timeTolerance_
                   << "\ndigis1 collection:     " << digis1_collection_
                   << "\ndigis2 collection:     " << digis2_collection_
                   << "\ndigis3 collection:     " << digis3_collection_
                   << "\nInput pass name:     " << passName_
                   << "\nOutput collection:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void TrigScintFirmwareTracker::produce(framework::Event &event) {

  if (verbose_) {
    ldmx_log(debug)
        << "TrigScintFirmwareTracker: produce() starts! Event number: "
        << event.getEventHeader().getEventNumber();
  }
  //I AM FILLING IN THE TRACKING LUT FOR LATER USE
  ap_int<12> A[3]={0,0,0};
  ap_int<12> LOOKUP[NCENT][COMBO][2];
  for(int i = 0; i<NCENT;i++){
    for(int j = 0; j<COMBO; j++){
      LOOKUP[i][j][0]=(i-A[1]+A[0]);LOOKUP[i][j][1]=(i-A[2]+A[0]);
      if(j/3==0){
	LOOKUP[i][j][0]-=1;
      }else if(j/3==2){
	LOOKUP[i][j][0]+=1;
      }
      if(j%3==0){
        LOOKUP[i][j][1]-=1;
      }else if(j%3==2){
	LOOKUP[i][j][1]+=1;
      }
      if(not((LOOKUP[i][j][0]>=0)and(LOOKUP[i][j][1]>=0)and(LOOKUP[i][j][0]<NCENT)and(LOOKUP[i][j][1]<NCENT))){
	LOOKUP[i][j][0]=-1;LOOKUP[i][j][1]=-1;
      }
    }
  }
  // looper over digi hits and aggregate energy depositions for each detID

  Hit HPad1[NHITS];
  Hit HPad2[NHITS];
  Hit HPad3[NHITS];

  Cluster Pad1[NCLUS];
  Cluster Pad2[NCLUS];
  Cluster Pad3[NCLUS];
  Track outTrk[NTRK];
  
  for(int j = 0; j<NHITS; j++){
    clearHit(HPad1[j]);
    clearHit(HPad2[j]);
    clearHit(HPad3[j]);
  }
  for(int j = 0; j<NCLUS; j++){
    if(j<NTRK){
      clearClus(Pad1[j]);
    }
    clearClus(Pad2[j]);
    clearClus(Pad3[j]);
  }
  for(int j = 0; j<NTRK; j++){
    clearTrack(outTrk[j]);
  }
  
  //std::cout<<"I GOT HERE 1"<<std::endl;
  const auto digis1_{
     event.getCollection<ldmx::TrigScintHit>(digis1_collection_, passName_)};
  const auto digis3_{
     event.getCollection<ldmx::TrigScintHit>(digis2_collection_, passName_)};
  const auto digis2_{
     event.getCollection<ldmx::TrigScintHit>(digis3_collection_, passName_)};

  std::cout<<"I GOT HERE 2"<<std::endl;
  // TODO remove this once verified that the noise overlap bug is gone

  int occupied[50];
  for(int i = 0; i<50;i++){
    occupied[i]=-1;
  }
  int count=0;
  for (const auto &digi : digis1_) {
    std::cout<<"I AM LOOPING IN DIGIS1 "<<digi.getPE()<<" "<<digi.getTime()<<std::endl;
    if ((digi.getPE() >
      minThr_)and(digi.getBarID()<=50)and(digi.getBarID()>=0)){//and(digi.getTime()>padTime_+timeTolerance_)) {
      std::cout<<"I AM LOOPING IN DIGIS1 "<<digi.getPE()<<" "<<digi.getTime()<<std::endl;
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index=count;
      if(occupied[(int)digi.getBarID()]>=0){
        if(HPad1[(int)occupied[(int)digi.getBarID()]].Amp<digi.getPE()){
	  HPad1[(int)occupied[(int)digi.getBarID()]].bID=(ap_int<12>)(digi.getBarID());
	  HPad1[(int)occupied[(int)digi.getBarID()]].mID=(ap_int<12>)(digi.getModuleID());
	  HPad1[(int)occupied[(int)digi.getBarID()]].Amp=(ap_int<12>)(digi.getPE());
	  HPad1[(int)occupied[(int)digi.getBarID()]].Time=(ap_int<12>)(digi.getTime());
	}
      }else{
	HPad1[count].bID=(ap_int<12>)(digi.getBarID());
	std::cout<<(ap_int<12>)(digi.getBarID())<<std::endl;
	HPad1[count].mID=(ap_int<12>)(digi.getModuleID());
	HPad1[count].Amp=(ap_int<12>)(digi.getPE());
	HPad1[count].Time=(ap_int<12>)(digi.getTime());
	occupied[digi.getBarID()]=count;
	count++;
      }
    }
  }

  std::cout<<"I GOT HERE 3"<<std::endl;
  for(int i = 0; i<50;i++){
    occupied[i]=-1;
  }
  count=0;
  for (const auto &digi : digis2_) {
    if ((digi.getPE() >
      minThr_)and(digi.getBarID()<=50)and(digi.getBarID()>=0)){//and(digi.getTime()>padTime_+timeTolerance_)) {
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index=count;
      if(occupied[(int)digi.getBarID()]>=0){
        if(HPad2[(int)occupied[(int)digi.getBarID()]].Amp<digi.getPE()){
	  HPad2[(int)occupied[(int)digi.getBarID()]].bID=(ap_int<12>)(digi.getBarID());
	  HPad2[(int)occupied[(int)digi.getBarID()]].mID=(ap_int<12>)(digi.getModuleID());
	  HPad2[(int)occupied[(int)digi.getBarID()]].Amp=(ap_int<12>)(digi.getPE());
	  HPad2[(int)occupied[(int)digi.getBarID()]].Time=(ap_int<12>)(digi.getTime());
	}
      }else{
	HPad2[count].bID=(ap_int<12>)(digi.getBarID());
	HPad2[count].mID=(ap_int<12>)(digi.getModuleID());
	HPad2[count].Amp=(ap_int<12>)(digi.getPE());
	HPad2[count].Time=(ap_int<12>)(digi.getTime());
	occupied[digi.getBarID()]=count;
	count++;
      }
    }
  }
  //std::cout<<"I GOT HERE 4"<<std::endl;
  for(int i = 0; i<50;i++){
    occupied[i]=-1;
  }
  count=0;
  for (const auto &digi : digis3_) {
    if ((digi.getPE() >
      minThr_)and(digi.getBarID()<=50)and(digi.getBarID()>=0)){//and(digi.getTime()>padTime_+timeTolerance_)) {
      ap_int<12> bID = (ap_int<12>)(digi.getBarID());
      ap_int<12> Amp = (ap_int<12>)(digi.getPE());
      int index=count;
      if(occupied[(int)digi.getBarID()]>=0){
        if(HPad3[(int)occupied[(int)digi.getBarID()]].Amp<digi.getPE()){
	  HPad3[(int)occupied[(int)digi.getBarID()]].bID=(ap_int<12>)(digi.getBarID());
	  HPad3[(int)occupied[(int)digi.getBarID()]].mID=(ap_int<12>)(digi.getModuleID());
	  HPad3[(int)occupied[(int)digi.getBarID()]].Amp=(ap_int<12>)(digi.getPE());
	  HPad3[(int)occupied[(int)digi.getBarID()]].Time=(ap_int<12>)(digi.getTime());
	}
      }else{
	HPad3[count].bID=(ap_int<12>)(digi.getBarID());
	HPad3[count].mID=(ap_int<12>)(digi.getModuleID());
	HPad3[count].Amp=(ap_int<12>)(digi.getPE());
	HPad3[count].Time=(ap_int<12>)(digi.getTime());
	occupied[digi.getBarID()]=count;
	count++;
      }
    }
  }
  count=0;
  std::cout<<"GOT HERE"<<std::endl;

  for(int J=0;J<NHITS;J++){
    std::cout<<HPad1[J].bID<<" "<<HPad1[J].Amp<<std::endl;
    std::cout<<HPad2[J].bID<<" "<<HPad2[J].Amp<<std::endl;
    std::cout<<HPad3[J].bID<<" "<<HPad3[J].Amp<<"\n"<<std::endl;
  }

  int counterN=0;
  Cluster* Point1=clusterproducer_sw(HPad1);
  int topSeed=0;
  for(int i = 0; i<NCLUS; i++){
    //bool Cond=(std::abs(Point1[i].Seed.bID-Point1[i].Sec.bID)<10)or(Point1[1].Sec.bID==-1)or(Point1[1].Sec.bID<50);
    std::cout<<Point1[i].Seed.Amp<<" "<<Point1[i].Seed.bID<<" "<<Point1[i].Sec.Amp<<" "<<Point1[i].Sec.bID<<std::endl;
    if((Point1[i].Seed.Amp<450)and(Point1[i].Seed.Amp>30)and(Point1[i].Seed.bID<51)and(Point1[i].Seed.bID>=0)and(Point1[i].Sec.Amp<450)and(counterN<NTRK)){
      std::cout<<Point1[i].Seed.Amp<<" "<<Point1[i].Seed.bID<<" "<<Point1[i].Sec.Amp<<" "<<Point1[i].Sec.bID<<"\n"<<std::endl;
      if(Point1[i].Seed.bID>=topSeed){
      	cpyHit(Pad1[counterN].Seed,Point1[i].Seed);cpyHit(Pad1[counterN].Sec,Point1[i].Sec);
      	calcCent(Pad1[counterN]);
      	counterN++;
	topSeed=Point1[i].Seed.bID;
      }
    }
  }
  Cluster* Point2=clusterproducer_sw(HPad2);
  topSeed=0;
  for(int i = 0; i<NCLUS; i++){
    //bool Cond=(std::abs(Point2[i].Seed.bID-Point2[i].Sec.bID)<10);
    std::cout<<Point2[i].Seed.Amp<<" "<<Point2[i].Seed.bID<<" "<<Point2[i].Sec.Amp<<" "<<Point2[i].Sec.bID<<std::endl;
    if((Point2[i].Seed.Amp<450)and(Point2[i].Seed.Amp>30)and(Point2[i].Seed.bID<51)and(Point2[i].Seed.bID>=0)and(Point2[i].Sec.Amp<450)){
      if(Point2[i].Seed.bID>=topSeed){
      	std::cout<<Point2[i].Seed.Amp<<" "<<Point2[i].Seed.bID<<" "<<Point2[i].Sec.Amp<<" "<<Point2[i].Sec.bID<<"\n"<<std::endl;
      	cpyHit(Pad2[i].Seed,Point2[i].Seed);cpyHit(Pad2[i].Sec,Point2[i].Sec);
      	calcCent(Pad2[i]);
	topSeed=Point2[i].Seed.bID;
      }
    }
  }
  Cluster* Point3=clusterproducer_sw(HPad3);
  topSeed=0;
  for(int i = 0; i<NCLUS; i++){
    //bool Cond=(std::abs(Point3[i].Seed.bID-Point3[i].Sec.bID)<10);
    std::cout<<Point3[i].Seed.Amp<<" "<<Point3[i].Seed.bID<<" "<<Point3[i].Sec.Amp<<" "<<Point3[i].Sec.bID<<std::endl;
    if((Point3[i].Seed.Amp<450)and(Point3[i].Seed.Amp>30)and(Point3[i].Seed.bID<51)and(Point3[i].Seed.bID>=0)and(Point3[i].Sec.Amp<450)){
      std::cout<<"Top Seed "<<topSeed<<std::endl;
      if(Point3[i].Seed.bID>=topSeed){
      	std::cout<<Point3[i].Seed.Amp<<" "<<Point3[i].Seed.bID<<" "<<Point3[i].Sec.Amp<<" "<<Point3[i].Sec.bID<<"\n"<<std::endl;
      	cpyHit(Pad3[i].Seed,Point3[i].Seed);cpyHit(Pad3[i].Sec,Point3[i].Sec);
      	calcCent(Pad3[i]);
	topSeed=Point3[i].Seed.bID;
      }
    }
  }

   if (verbose_) {
    ldmx_log(debug) << "Got digi collection " << digis1_collection_ << "_"
                    << passName_ << " with " << digis1_.size() << " entries ";
  }
  std::cout<<"I GOT HERE 5"<<std::endl;
  trackproducer_hw(Pad1,Pad2,Pad3,outTrk,LOOKUP);
  std::cout<<"I GOT HERE 6"<<std::endl;
  for(int I = 0; I<NTRK; I++){
    if(outTrk[I].Pad1.Seed.Amp>0){
      std::cout<<outTrk[I].Pad1.Seed.bID<<" "<<outTrk[I].Pad2.Seed.bID<<" "<<outTrk[I].Pad3.Seed.bID<<std::endl;
      ldmx::TrigScintTrack trk = makeTrack(outTrk[I]);
      tracks_.push_back(trk);
    }
  }
  event.add(output_collection_, tracks_);
  tracks_.resize(0);
  return;
}

ldmx::TrigScintTrack TrigScintFirmwareTracker::makeTrack(Track outTrk) {
  ldmx::TrigScintTrack tr;
  float pe = outTrk.Pad1.Seed.Amp+outTrk.Pad1.Sec.Amp;
  pe += outTrk.Pad2.Seed.Amp+outTrk.Pad2.Sec.Amp;
  pe += outTrk.Pad3.Seed.Amp+outTrk.Pad3.Sec.Amp;
  tr.setCentroid(calcTCent(outTrk));
  calcResid(outTrk);
  tr.setPE(pe);
  return tr;
}

void TrigScintFirmwareTracker::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigScintFirmwareTracker::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintFirmwareTracker);
