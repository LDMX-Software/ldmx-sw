/**
 * @file TrigScintTrackProducer.cxx
 * @brief 
 * @author Lene Kristian Bryngemark, Stanford University 
 */

#include "EventProc/TrigScintTrackProducer.h"


#include <iterator>     // std::next
#include <map> 


namespace ldmx {
  
  void TrigScintTrackProducer::configure(Parameters& ps) {

    maxDelta_                = ps.getParameter< double >("delta_max");  // max distance to consider adding in a cluster to track
    seeding_collection_      = ps.getParameter< std::string >("seeding_collection"); //probably tagger pad, "TriggerPadTagClusters" 
    input_collections_       = ps.getParameter< std::vector < std::string > >("further_input_collections"); // {"TriggerPadUpClusters" , "TriggerPadDnClusters" }
    output_collection_       = ps.getParameter< std::string >("output_collection"); 
    passName_                = ps.getParameter< std::string >("input_pass_name"); 
    verbose_                 = ps.getParameter< int >("verbosity");

    // TO DO: allow any number of input collections
    

    if (verbose_)
      {
	ldmx_log(info)<< "In TrigScintTrackProducer: configure done!" << std::endl;
	ldmx_log(info)<< "Got parameters: \nSeeding:   " << seeding_collection_ 
		      << "\nTolerance: " << maxDelta_
		      << "\nInput:     " << input_collections_.at(0) << " and " <<  input_collections_.at(1)
		      << "\nInput pass name:     " << passName_ 
		      << "\nOutput:    " << output_collection_
		      << "\nVerbosity: " << verbose_ ;
      }
    

    return;
  }
  
  void TrigScintTrackProducer::produce(ldmx::Event& event) {
 
    //parameters.
    // one pad cluster collection to use as seed
    // a vector with the other two 
    // a maximum distance between seed centroid and other pad clusters to go to the same track
    // an output collection name
    // a verbosity controller
    
    if ( verbose_) {
      ldmx_log(debug)<< "TrigScintTrackProducer: produce() starts! Event number: " << event.getEventHeader().getEventNumber();
      
    }
    
    
    const auto seeds{event.getCollection< TrigScintCluster >(seeding_collection_, passName_)}; 
    uint numSeeds = seeds.size() ; 
    
    if ( verbose_) {
      ldmx_log(debug)<< "Got track seeding cluster collection " << seeding_collection_ << " with " << numSeeds << " entries ";
    }
    
    
    if (! event.exists( input_collections_.at(0) )) {
      ldmx_log(info)<< "No collection called " << input_collections_.at(0) << "; still, not skipping event";
    }
    const auto clusters_pad1{event.getCollection< TrigScintCluster >(input_collections_.at(0), passName_)}; 
    
    
    if (! event.exists( input_collections_.at(1) )) {
      ldmx_log(info)<< "No collection called " << input_collections_.at(1) << "; still, not skipping event";
    }
    
    const auto clusters_pad2{event.getCollection< TrigScintCluster >(input_collections_.at(1), passName_)}; 
    
    if ( verbose_) {
      ldmx_log(debug)<< "Got the other two pad collections:" << input_collections_.at(0)  << " with " << clusters_pad1.size() << " entries, and " 
		     << input_collections_.at(1)  << " with " << clusters_pad2.size() << " entries.";
    }
    
    std::vector < TrigScintTrack > cleanedTracks;

    // loop over the clusters in the seeding pad collection, if there are clusters in all pads 
    if ( numSeeds && clusters_pad1.size()  && clusters_pad2.size() )  {
      
      for (const auto& seed : seeds) {
	// for each seed, search through the other two pads to match all clusters with centroids within tolerance to tracks 
	float centroid = seed.getCentroid();
	
	std::vector < TrigScintTrack > trackCandidates;
	
	if ( verbose_ > 1 ) {
	  ldmx_log(debug)<< "Got seed with centroid " << centroid;
	}

	//reset for each seed 
	bool madeTrack = false;
		  
	for (const auto& cluster1 : clusters_pad1) {
	  
	  if ( verbose_ > 1 ) {
	    ldmx_log(debug)<< "\tGot pad1 cluster with centroid " << cluster1.getCentroid();
	  }
	  if (  fabs(cluster1.getCentroid() - centroid ) < maxDelta_ ) { // first match! loop through next pad too 
	    
	    if ( verbose_ > 1 ) {
	      ldmx_log(debug)<< "\t\tIt is close enough!. Check pad2";
	    }
	    
	    for (const auto& cluster2 : clusters_pad2) {
	      if ( verbose_ > 1 ) {
		ldmx_log(debug)<< "\tGot pad2 cluster with centroid " << cluster2.getCentroid();
	      }
	      
	      if (  fabs(cluster2.getCentroid() - centroid ) < maxDelta_ ) { // first match! loop through next pad too 

		if ( verbose_ > 1 ) {
		  ldmx_log(debug)<< "\t\tIt is close enough!. Make a track";
		}
		
		//only make this vector now! this ensures against hanging clusters with indices from earlier in the loop 
		std::vector < TrigScintCluster > clusterVec = {seed, cluster1, cluster2};

		//make a track 
		TrigScintTrack track = makeTrack( clusterVec );
		trackCandidates.push_back( track );

		/*
		// here we could break if we didn't want to allow all possible combinations
		madeTrack=true;
		break; //we're done with this iteration once there's a track made 
		*/
				  
	      }//if match in pad2
	    }//over clusters in pad2
	  }//if match in pad1
	  /*
	  //same here
	  if (madeTrack)
	  break;
	  */
			
	}//over clusters in pad1
	
	
	// continue to next seed if 0 track candidates 
	if ( trackCandidates.size() == 0 )
	  continue;

	int keepIdx=0;
	float minResidual = 1000; //some large number
	

	//no need to choose between only one candidate track 
	if ( trackCandidates.size() > 1 ) {
	  // now for each seed, pick only the track with the smallest residual. 
	  
	  if ( verbose_  ) {
	    ldmx_log(debug)<< "Got " << trackCandidates.size() << " tracks to check.";
	  }
	  
	  for (uint idx=0; idx < trackCandidates.size(); idx++){
	    if ( (trackCandidates.at(idx)).getResidual() < minResidual ) {
	      keepIdx = (int)idx;
	      minResidual = (trackCandidates.at(idx)).getResidual() ; //update minimum 
	      
	      if (verbose_ > 1 ) {
		ldmx_log(debug)<< "Track at index " << idx << " has smallest residual so far: " << minResidual;
	      }
	      
	    }//finding min residual 
	  }//over track candidates
	}//if more than 1 to choose from
	
	// store the track at keepIdx, if there was one we made it this far and keepIdx is 0 or has been updated to the smallest residual track idx
	//	if (keepIdx >= 0) {
	tracks_.push_back( trackCandidates.at(keepIdx) );
	if (verbose_ ) {
	  ldmx_log(debug)<< "Kept track at index " << keepIdx;
	  (trackCandidates.at(keepIdx)).Print();
	}
	//}
      } // over seeds 

      //done here if there were no tracks found
      if (tracks_.size() == 0) {
	if (verbose_ ) {
	  ldmx_log(debug)<< "No tracks found!";
	}
	return;
      }		
      // now, if there are multiple seeds sharing the same downstream hits, this should also be remedied with a selection on min residual.

      // The logic of this loop kind of assumes I can remove tracks immediately -- that way I can do pairwise checks between more tracks within a single loop.
      // But for now I haven't figured out how to erase elements in a fool proof way. So I iterate over a vector...

      std::vector keepIndices( tracks_.size(), 1);
      if (verbose_ > 1)
	ldmx_log(debug)<< "vector of indices to keep has size " << keepIndices.size();
		
      for (uint idx=tracks_.size()-1; idx > 0 ;idx--){

	TrigScintTrack track = tracks_.at( idx );
	TrigScintTrack nextTrack = tracks_.at( idx-1);
	if (verbose_ > 1)
	  ldmx_log(debug)<< "In track disambiguation loop, idx points at " << idx << " and prev idx points at " << idx-1;

	std::vector<TrigScintCluster> consts_1 = track.getConstituents();
	std::vector<TrigScintCluster> consts_2 = nextTrack.getConstituents();
	if (verbose_ > 1)
	  ldmx_log(debug)<< "In track disambiguation loop, got the two tracks, with nConstituents " << consts_1.size() << " and " << consts_2.size() << ", respectively. ";
	// let's do "if either cluster is shared" right now... but could also have it settable to use a stricter cut: an AND 
	if ( consts_1[1].getCentroid() ==  consts_2[1].getCentroid() || consts_1[2].getCentroid() == consts_2[2].getCentroid() ) { //we have overlap downstream of the seeding pad. probably, one cluster in seeding pad is noise
			
	  if (verbose_ > 1 ) {
	    ldmx_log(debug)<< "Found overlap! Tracks at index " << idx << " and " << idx -1;
	    (tracks_.at(idx)).Print();
	    (tracks_.at(idx-1)).Print();
	  }
			
	  if ( (tracks_.at(idx)).getResidual() < (tracks_.at(idx-1)).getResidual() ) {
	    //next track (lower index) is a worse choice, remove its flag for keeping 
	    keepIndices.at(idx-1)=0;
	  }
	  else //prefer next track over current. remove current track's keep flag
	    keepIndices.at(idx)=0;
	  /*}
	    else {
	    tracks_.erase(itNext);
	    //        removeIdx.push_back(idx+1);
	    // we might see the same index two times in the loop in this case, if there are three seeds sharing the same clusters downstream.
	    // then the third only gets removed if it's even worse than the second.
	    // one could deal with this with an extra overlap check. not sure we will be in this situation any time soon though.
	    }*/
	}// over matching/overlapping tracks
      }// over constructed tracks
	   
      for (uint idx=0; idx<tracks_.size(); idx++){
	if (verbose_ > 1 ) {
	  ldmx_log(debug)<< "keep flag for idx " << idx << " is " << keepIndices.at(idx);
	}
	if ( keepIndices.at(idx) ) { //this hasn't been flagged for removal
			
	  cleanedTracks.push_back( tracks_.at(idx) );
			
	  if (verbose_ ) {
	    ldmx_log(debug)<< "After cleaning, keeping track at index " << idx << ":";
	    (tracks_.at(idx)).Print();
	  }
	}//if index flagged for keeping
      }//over all (uniquely seeded) tracks in the event
      /*
	if (verbose_ ) {
	for (uint idx=0; idx < tracks_.size(); idx++){
	ldmx_log(debug)<< "Keeping track at index " << idx << ":";
	(tracks_.at(idx)).Print();
	}
	}
      */
	  
    } // if there are clusters in all pads 
    else 
      if (verbose_ ) {
	ldmx_log(info)<< "Not all pads had clusters; skipping tracking attempt";
      }
    

    if (verbose_ ) {
      ldmx_log(debug)<< "Done with tracking step. "  ;
    }

    event.add(output_collection_, cleanedTracks);
    tracks_.resize(0);

    return;
  }
    
  
  TrigScintTrack TrigScintTrackProducer::makeTrack( std::vector<TrigScintCluster> clusters ) {
    //for now let's keep a straight, unweighted centroid
    // consider the possibility that at least one cluster has a centroid identically == 0. 
    // then we need to shift them by 1 if we want to do energy weighted track centroid later. but no need now
    TrigScintTrack tr;
    float centroid = 0;
    float beamEfrac = 0;
    for (uint i = 0; i< clusters.size() ; i++) {
      centroid+=(clusters.at(i)).getCentroid();
      tr.addConstituent( clusters.at(i) );
      beamEfrac+=(clusters.at(i)).getBeamEfrac();
    }
    centroid/=clusters.size();
    beamEfrac/=clusters.size();

    float residual = 0;
    for (uint i = 0; i< clusters.size() ; i++)
      residual+= ( (clusters.at(i)).getCentroid() - centroid )*( (clusters.at(i)).getCentroid() - centroid );
    residual=sqrt( (float)(residual/clusters.size()) );
  
    tr.setCentroid( centroid );
    tr.setResidual( residual );
    tr.setBeamEfrac( beamEfrac );
  

    if (verbose_ ) {
      ldmx_log(debug)<< " --  In makeTrack made track with centroid  " << centroid << " and residual " << residual << " from clusters with centroids";
      for (uint i = 0; i< clusters.size() ; i++)
	ldmx_log(debug)<< "\tpad " << i << ": centroid " << (clusters.at(i)).getCentroid() ;
      ldmx_log(debug) ;
    }
  
    return tr;
  
  
  }

  
  void TrigScintTrackProducer::onFileOpen() {

    ldmx_log(debug)<< "Opening file!";
	  
    return;
  }
  
  
  void TrigScintTrackProducer::onFileClose() {
	  
    ldmx_log(debug)<< "Closing file!";
	  
    return;
  }
  
  void TrigScintTrackProducer::onProcessStart() {

    ldmx_log(debug)<< "Process starts!";

    return;
  }

  void TrigScintTrackProducer::onProcessEnd() {
    
    ldmx_log(debug)<< "Process ends!";
    
    return;
  }
  
}

DECLARE_PRODUCER_NS(ldmx, TrigScintTrackProducer);
