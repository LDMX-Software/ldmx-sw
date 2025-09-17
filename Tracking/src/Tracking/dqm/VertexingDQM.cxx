#include "Tracking/dqm/VertexingDQM.h"

#include <algorithm>
#include <iostream>

#include "Tracking/Sim/TrackingUtils.h"
#include "SimCore/Event/SimParticle.h"


namespace tracking::dqm {
  
  void VertexingDQM::configure(framework::config::Parameters& parameters) {
    vertexCollection_ = parameters.getParameter<std::string>("vertex_collection");
    truthCollection_ = parameters.getParameter<std::string>("truth_collection");
    trackCollection_ = parameters.getParameter<std::string>("track_collection");
    
    target_sp_events_passname_ =
      parameters.getParameter<std::string>("target_sp_events_passname");
    target_sp_passname_ =
      parameters.getParameter<std::string>("target_sp_passname");
    truth_passname_ = parameters.getParameter<std::string>("truth_passname");
    truth_events_passname_ =
      parameters.getParameter<std::string>("truth_events_passname");
    track_passname_ = parameters.getParameter<std::string>("track_passname");
    track_events_passname_ =
      parameters.getParameter<std::string>("track_events_passname");
    simparticle_passname_ = parameters.getParameter<std::string>("simparticle_passname");
    vertex_collection_events_passname_ = parameters.getParameter<std::string>("vertex_events_passname");
    
    title_ = parameters.getParameter<std::string>("title", "recoil_vertex_");
    trackProb_cut_ = parameters.getParameter<double>("trackProb_cut", 0.5);
    subdetector_ = parameters.getParameter<std::string>("subdetector", "Recoil");
    
    ldmx_log(info) << "Vertex Collection " << vertexCollection_ << std::endl;
    ldmx_log(info) << "Track Collection " << trackCollection_ << std::endl;
    ldmx_log(info) << "Truth Collection " << truthCollection_ << std::endl;
    
    pidmap[-321] = PIDBins::kminus;
    pidmap[321] = PIDBins::kplus;
    pidmap[-211] = PIDBins::piminus;
    pidmap[211] = PIDBins::piplus;
    pidmap[11] = PIDBins::electron;
    pidmap[-11] = PIDBins::positron;
    pidmap[2212] = PIDBins::proton;
    pidmap[-2212] = PIDBins::antiproton;
  }
  
  void VertexingDQM::analyze(const framework::Event& event) {
    ldmx_log(debug) << "DQM Reading in::" << trackCollection_ << std::endl;
    
    if (!event.exists(vertexCollection_, vertex_collection_events_passname_)) {
      ldmx_log(error) << "ERROR:: vertexCollection " << vertexCollection_
		      << " for pass  = "<<vertex_collection_events_passname_
		      << " not in event" << std::endl;
      return;
    }
    
    auto vertices{
      event.getCollection<ldmx::Vertex>(vertexCollection_,  vertex_collection_events_passname_)};
    
    if (!event.exists(trackCollection_, track_events_passname_)) {
      ldmx_log(error) << "ERROR:: trackCollection " << trackCollection_
		      << " not in event" << std::endl;
    return;
    }
    //    auto tracks{ event.getCollection<ldmx::Track>(trackCollection_, track_passname_)};
    if (event.exists(trackCollection_, track_events_passname_)) {
      recoTrackCollection_ = std::make_shared<ldmx::Tracks>(event.getCollection<ldmx::Track>(trackCollection_, track_passname_));
    }
    
      // The truth track collection
    if (event.exists(truthCollection_, truth_events_passname_)) {
      truthTrackCollection_ = std::make_shared<ldmx::Tracks>(event.getCollection<ldmx::Track>(truthCollection_, truth_passname_));
      doTruthComparison = true;
    }
    
    if (event.exists("TargetScoringPlaneHits", target_sp_events_passname_)) {
      target_scoring_hits_ =
	std::make_shared<std::vector<ldmx::SimTrackerHit>>(event.getCollection<ldmx::SimTrackerHit>("TargetScoringPlaneHits",target_sp_passname_));
    }
    
    auto particleMap{event.getMap<int, ldmx::SimParticle>("SimParticles", simparticle_passname_)};
    auto simTrackerHits{event.getCollection<ldmx::SimTrackerHit>("RecoilSimHits","")};
    std::cout<<"number of recoil sim tracker hits = "<<simTrackerHits.size()<<std::endl;

    //////////////////////////////
    //  this block looks for Ks->pi+pi- in the generated particle map
    //  while looking for findable (>5 sim hit) pions
    //  and fill some plots so we can do some accounting
    
    //make some counters
    int nKs=0;
    int nKsToPiPPiM=0;   
    int nFindableKs=0;   
    int nFoundKs=0;
    //print out the map, see if there is a Ks in the event
    for (auto it = particleMap.begin(); it != particleMap.end(); ++it) {
      //      std::cout << "ID number = " << it->first<<std::endl;
      //it->second.Print();
      if(it->second.getPdgID()!=kshortPdgId_)
	continue; 
      it->second.Print();
      std::cout << "Found a Kshort SimParticle with ID number = " << it->first <<std::endl;
      nKs++;
      if(abs(particleMap[it->second.getDaughters()[0]].getPdgID()) == pionPdgId_)//check if this is a decay to charged pions
	nKsToPiPPiM++;
      else
	continue;

      // at this point we found a generated Ks->pi+pi-; reset some counters and fill truth plots
      int nFindablePion=0;
      int nTruthFound=0;
      int nRecoFound=0;
      histograms_.fill("truth_Ks_origin_X", it->second.getVertex()[0]);
      histograms_.fill("truth_Ks_origin_Y", it->second.getVertex()[1]);
      histograms_.fill("truth_Ks_origin_Z", it->second.getVertex()[2]);
      histograms_.fill("truth_Ks_endpoint_X", it->second.getEndPoint()[0]);
      histograms_.fill("truth_Ks_endpoint_Y", it->second.getEndPoint()[1]);
      histograms_.fill("truth_Ks_endpoint_Z", it->second.getEndPoint()[2]);
      
      for(int iD=0; iD<it->second.getDaughters().size(); iD++){	
	int daugID= it->second.getDaughters()[iD];
	int daugPdgID = particleMap[daugID].getPdgID(); 
	if(abs(daugPdgID) == pionPdgId_){
	  std::cout<<"      Ks charged pion daughter  track ID = "<<daugPdgID<<std::endl;
	  particleMap[daugID].Print();
	  histograms_.fill("truth_pion_momentum_X", particleMap[daugID].getMomentum()[0]/1000.);
	  histograms_.fill("truth_pion_momentum_Y", particleMap[daugID].getMomentum()[1]/1000.);
	  histograms_.fill("truth_pion_momentum_Z", particleMap[daugID].getMomentum()[2]/1000.);
	  int chPionSTHCount = 0;	  
	  //only do this for charged pions	  
	  for (int iSt = 0; iSt<simTrackerHits.size(); iSt++){
	    ldmx::SimTrackerHit sth=simTrackerHits[iSt];
	    //	    std::cout<<"pdgID = "<< sth.getPdgID()<< "  trackID = "<<sth.getTrackID()<<std::endl;
	    if(abs(sth.getPdgID()) == pionPdgId_  && sth.getTrackID()==daugID){
	      std::cout<<"found a sim tracker hit from this sim particle on layer = "<<sth.getLayerID()<<"  at z = "<<sth.getPosition()[2]<<std::endl;
	      chPionSTHCount++; 
	    }
	  }
	  std::cout<<"Found "<<chPionSTHCount<<" sim tracker hits for this Kshort charged pion"<<std::endl;
	  histograms_.fill("N_Ks_pipi_pion_simtrkhits",chPionSTHCount); 
	  if(chPionSTHCount>5){//this pion is findable...fill some plots
	    std::cout<<"       ...should be enough for a track?"<<std::endl;
	    nFindablePion++; 
	    //fill findable pion plots
	    histograms_.fill("truth_findable_pion_momentum_X", particleMap[daugID].getMomentum()[0]/1000.);
	    histograms_.fill("truth_findable_pion_momentum_Y", particleMap[daugID].getMomentum()[1]/1000.);
	    histograms_.fill("truth_findable_pion_momentum_Z", particleMap[daugID].getMomentum()[2]/1000.);
	    ldmx::Track* truth_pion=getTrackFromSimParticleID(truthTrackCollection_,daugID);  //  this doesn't seem to be working!
	    if(truth_pion){
	      nTruthFound++;
	      std::cout<<"  found truth track for this pion"<<std::endl;	    
	    }
	    
	    ldmx::Track* reco_trk=getTrackFromSimParticleID(recoTrackCollection_,daugID); 
	    if(reco_trk){
	      histograms_.fill("truth_found_pion_momentum_X", particleMap[daugID].getMomentum()[0]/1000.);
	      histograms_.fill("truth_found_pion_momentum_Y", particleMap[daugID].getMomentum()[1]/1000.);
	      histograms_.fill("truth_found_pion_momentum_Z", particleMap[daugID].getMomentum()[2]/1000.);
	      nRecoFound++;
	      std::cout<<"  found reco track for this pion"<<std::endl;
	    }
	  }//if sth>5
	}//if daughter is pion+/-	
      }//loop over Ks daughters
      if(nFindablePion==2){
	//fill findable Ks plots
	nFindableKs+=1;
	histograms_.fill("truth_findable_Ks_origin_X", it->second.getVertex()[0]);
	histograms_.fill("truth_findable_Ks_origin_Y", it->second.getVertex()[1]);
	histograms_.fill("truth_findable_Ks_origin_Z", it->second.getVertex()[2]);
	histograms_.fill("truth_findable_Ks_endpoint_X", it->second.getEndPoint()[0]);
	histograms_.fill("truth_findable_Ks_endpoint_Y", it->second.getEndPoint()[1]);
	histograms_.fill("truth_findable_Ks_endpoint_Z", it->second.getEndPoint()[2]);
	if(nRecoFound==2){
	  histograms_.fill("truth_found_Ks_origin_X", it->second.getVertex()[0]);
	  histograms_.fill("truth_found_Ks_origin_Y", it->second.getVertex()[1]);
	  histograms_.fill("truth_found_Ks_origin_Z", it->second.getVertex()[2]);
	  histograms_.fill("truth_found_Ks_endpoint_X", it->second.getEndPoint()[0]);
	  histograms_.fill("truth_found_Ks_endpoint_Y", it->second.getEndPoint()[1]);
	  histograms_.fill("truth_found_Ks_endpoint_Z", it->second.getEndPoint()[2]);
	  nFoundKs++;
	}
      }
      histograms_.fill("N_Ks_pipi_pion_findable",nFindablePion);
      histograms_.fill("N_Ks_pipi_pion_truth_trks",nTruthFound);
      histograms_.fill("N_Ks_pipi_pion_reco_trks",nRecoFound);      
    }//loop over particle map
    histograms_.fill("N_Ks",nKs);
    histograms_.fill("N_Ks_to_piplus_piminus",nKsToPiPPiM);
    histograms_.fill("N_Ks_findable",nFindableKs);
    
    //If there was no Ks->pi+pi- in the simulation, bail out
    if(nKs==0)
      return;
    if(nKsToPiPPiM==0)
      return;

    // only look at "found"  Ks for now
    if(nFoundKs==0)
      return; 
    //   done with mc particle block
    /////////////////////////////////////
    
    
    ldmx_log(info) << "Filling general histograms " << std::endl;

    // General Plots       
    histograms_.fill("reco_N_tracks", recoTrackCollection_->size());
    histograms_.fill("reco_N_vertex", vertices.size());

    for(int iV=0; iV<vertices.size(); iV++){
      std::cout<<"Checking vertex #"<<iV<<std::endl;
      auto vert = vertices.at(iV);

      //select vertex that comes from Ks (mean decay length at 1GeV ~ 5.4cm)
      bool foundPiMinus=false;
      bool foundPiPlus=false;
      std::vector<ldmx::Track> inTrks=vert.getOriginalTracks();

      ///////////////////////
      //  this block looks for truth pions from the Ks
      //  ****   I don't think the truth track Id is working....skip for now
      //  use reco track instead (sorry, still call it truth track)
      for(int iT=0; iT<inTrks.size(); iT++){
	ldmx::Track track=inTrks.at(iT);
	// find the truth track matched to this reco track
	//	ldmx::Track* truth_trk=getTrackFromSimParticleID(truthTrackCollection_,track.getTrackID()); 
	//double trackTruthProb = track.getTruthProb();
	//if(!truth_trk||trackTruthProb<0.5){	  
	//  std::cout<<"couldn't find truth track"<<std::endl;
	//  continue;
	//}
	ldmx::Track* truth_trk=getTrackFromSimParticleID(recoTrackCollection_,track.getTrackID());
	if(!truth_trk){
	  std::cout<<"couldn't find truth track"<<std::endl;
	  continue;
	}
	if(truth_trk){
	  int truthID=truth_trk->getTrackID();	  
	  ldmx::SimParticle trkSim=particleMap[truthID];
	  if(abs(trkSim.getPdgID()) != pionPdgId_){
	    std::cout<<"truth track is not a pion pdgId = "<<trkSim.getPdgID()<<"  id = "<<truthID<<std::endl;
	    continue;
	  } else{
	    std::cout<<"one of the vertex tracks is a pion!  pdgId = "<<trkSim.getPdgID()<<"  id = "<<truthID<<std::endl;
	  }
	  std::vector<int> parents=trkSim.getParents();//vector of trackIds for the parents
	  int pSize=parents.size();
	  if(pSize!=1){
	    std::cout<<"parents list size = "<<pSize<<"  != 1 "<<std::endl;
	    continue; 
	  }
	  ldmx::SimParticle parent=particleMap[parents[0]];
	  int parentPdgID=parent.getPdgID();
	  int parentTrackID=parents[0];
	  
	  if(parentPdgID != kshortPdgId_){
	    std::cout<<"pion is not from Ks parent pdgId ="<<parentPdgID<<"  parent track Id = "<< parentTrackID <<std::endl;
	    continue;
	  }

	  //if I made it here, I found a pion from a Ks
	  //assign plus or minus as true
	  if(trkSim.getPdgID()>0)
	    foundPiPlus=true;
	  else
	    foundPiMinus=true; 
	  //////note....I should actually check that these are from SAME Ks!
	  
	}       
      } //end tracks on vertex

      //check if we found both pions
      if(!(foundPiPlus&&foundPiMinus)){
	std::cout<<"vertex is not from Ks...bailing"<<std::endl;
	continue; 
      }      
      //found pi+pi- from Ks
      std::cout<<" FOUND A pi+pi- from Ks!!!"<<std::endl;
      
      histograms_.fill("reco_ks_vertex_x",vert.position()[0]);
      histograms_.fill("reco_ks_vertex_y",vert.position()[1]);
      histograms_.fill("reco_ks_vertex_z",vert.position()[2]);
      histograms_.fill("reco_ks_vertex_px",vert.momentum()[0]);
      histograms_.fill("reco_ks_vertex_py",vert.momentum()[1]);
      histograms_.fill("reco_ks_vertex_pz",vert.momentum()[2]);
      histograms_.fill("reco_ks_vertex_mass",vert.getMass());
      histograms_.fill("reco_ks_vertex_mass_near_Ks",vert.getMass());


      histograms_.fill("reco_ks_vertex_chi2",vert.getChi2());
      histograms_.fill("reco_ks_vertex_ndf",vert.getNDF());


      for(int iT=0; iT<inTrks.size(); iT++){
	ldmx::Track trk=inTrks.at(iT);
	histograms_.fill("reco_ks_vertex_N_hits",trk.getNhits());
	histograms_.fill("reco_ks_pion_momentum_X",trk.getMomentum()[1]);
	histograms_.fill("reco_ks_pion_momentum_Y",trk.getMomentum()[2]);
	histograms_.fill("reco_ks_pion_momentum_Z",trk.getMomentum()[0]);
      }
      std::vector<ldmx::Vertex::FittedTrack> fitTracks=vert.getFittedTracks();
      for(int iT=0; iT<fitTracks.size(); iT++){
	ldmx::Vertex::FittedTrack fitTrk=fitTracks.at(iT);
	histograms_.fill("fitted_ks_pion_momentum_X",fitTrk.momentum[0]);
	histograms_.fill("fitted_ks_pion_momentum_Y",fitTrk.momentum[1]);
	histograms_.fill("fitted_ks_pion_momentum_Z",fitTrk.momentum[2]);
	
      }

    }
    //just fill histograms from all V0s
    
    for(int iV=0; iV<vertices.size(); iV++){
      auto vert = vertices.at(iV);
      histograms_.fill("reco_all_vertex_x",vert.position()[0]);
      histograms_.fill("reco_all_vertex_y",vert.position()[1]);
      histograms_.fill("reco_all_vertex_z",vert.position()[2]);
      histograms_.fill("reco_all_vertex_px",vert.momentum()[0]);
      histograms_.fill("reco_all_vertex_py",vert.momentum()[1]);
      histograms_.fill("reco_all_vertex_pz",vert.momentum()[2]);
      histograms_.fill("reco_all_vertex_mass",vert.getMass());
      histograms_.fill("reco_all_vertex_mass_near_Ks",vert.getMass());


      histograms_.fill("reco_all_vertex_chi2",vert.getChi2());
      histograms_.fill("reco_all_vertex_ndf",vert.getNDF());

      std::vector<ldmx::Track> inTrks=vert.getOriginalTracks();
      for(int iT=0; iT<inTrks.size(); iT++){
	ldmx::Track trk=inTrks.at(iT);
	histograms_.fill("reco_all_vertex_N_hits",trk.getNhits());
	histograms_.fill("reco_all_track_momentum_X",trk.getMomentum()[1]);
	histograms_.fill("reco_all_track_momentum_Y",trk.getMomentum()[2]);
	histograms_.fill("reco_all_track_momentum_Z",trk.getMomentum()[0]);
      }
      std::vector<ldmx::Vertex::FittedTrack> fitTracks=vert.getFittedTracks();
      for(int iT=0; iT<fitTracks.size(); iT++){
	ldmx::Vertex::FittedTrack fitTrk=fitTracks.at(iT);
	histograms_.fill("fitted_all_track_momentum_X",fitTrk.momentum[0]);
	histograms_.fill("fitted_all_track_momentum_Y",fitTrk.momentum[1]);
	histograms_.fill("fitted_all_track_momentum_Z",fitTrk.momentum[2]);
	
      }
    }
  }
  
  void VertexingDQM::onProcessEnd() {
    // Produce the efficiency plots. (TODO::Switch to TEfficiency instead)
  }


  ldmx::Track*  VertexingDQM::getTrackFromSimParticleID(std::shared_ptr<ldmx::Tracks> trk_coll, int trkID){
    //see if track was found in truth tracking:
    
    ldmx::Track* trk = nullptr;
    auto it = std::find_if(trk_coll->begin(),
			   trk_coll->end(),
			   [&](const ldmx::Track& tt) {
			     return tt.getTrackID() == trkID;
			   });
    if (it != trk_coll->end()) 
      trk = &(*it);

    return trk;
  }

  
}// namespace tracking::dqm



DECLARE_ANALYZER(tracking::dqm::VertexingDQM)
