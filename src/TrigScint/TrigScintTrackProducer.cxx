#include "TrigScint/TrigScintTrackProducer.h"

#include <iterator>  // std::next
#include <map>

namespace trigscint {

void TrigScintTrackProducer::configure(framework::config::Parameters &ps) {
  maxDelta_ = ps.getParameter<double>(
      "delta_max");  // max distance to consider adding in a cluster to track
  seeding_collection_ = ps.getParameter<std::string>(
      "seeding_collection");  // probably tagger pad, "TriggerPadTagClusters"
  input_collections_ = ps.getParameter<std::vector<std::string>>(
      "further_input_collections");  // {"TriggerPadUpClusters" ,
                                     // "TriggerPadDnClusters" }
  output_collection_ = ps.getParameter<std::string>("output_collection");
  passName_ = ps.getParameter<std::string>("input_pass_name");
  verbose_ = ps.getParameter<int>("verbosity");
  vertBarStartIdx_ = ps.getParameter<int>("vertical_bar_start_index");
  nBarsY_ = ps.getParameter<int>("number_horizontal_bars");
  barWidth_y_ = ps.getParameter<double>("horizontal_bar_width");
  barGap_y_ = ps.getParameter<double>("horizontal_bar_gap");
  nBarsX_ = ps.getParameter<int>("number_vertical_bars");
  barWidth_x_ = ps.getParameter<double>("vertical_bar_width");
  barGap_x_ = ps.getParameter<double>("vertical_bar_gap");
  skipLast_ = ps.getParameter<bool>("allow_skip_last_collection");

  
  // TO DO: allow any number of input collections

  if (verbose_) {
    ldmx_log(info) << "In TrigScintTrackProducer: configure done!" << std::endl;
    ldmx_log(info) << "Got parameters: \nSeeding:   " << seeding_collection_
                   << "\nTolerance: " << maxDelta_
                   << "\nInput:     " << input_collections_.at(0) << " and "
                   << input_collections_.at(1)
                   << "\nInput pass name:     " << passName_
                   << "\nAllow tracks with no hit in last collection:     " << skipLast_
                   << "\nVertical bar start index:     " << vertBarStartIdx_
                   << "\nNumber of horizontal bars:     " << nBarsY_
                   << "\nHorizontal bar width:     " << barWidth_y_
                   << "\nHorizontal bar gap:     " << barGap_y_
                   << "\nNumber of vertical bars:     " << nBarsX_
                   << "\nVertical bar width:     " << barWidth_x_
                   << "\nVertical bar gap:     " << barGap_x_
                   << "\nOutput:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }


  yConvFactor_ = (barWidth_y_+barGap_y_)/2.; //each bar only goes half this distance up (overlap/zig-zag)
  yStart_ = -(nBarsY_*(barWidth_y_+barGap_y_) - barGap_y_)/2.; //half height of pad
  xConvFactor_ = barWidth_x_+barGap_x_; //each bar goes entire distance sideways (no overlap)
  xStart_ =-(nBarsX_*(barWidth_x_+barGap_x_) - barGap_x_)/2.; //half width of pad

  return;
}

void TrigScintTrackProducer::produce(framework::Event &event) {
  // parameters.
  // one pad cluster collection to use as seed
  // a vector with the other two
  // a maximum distance between seed centroid and other pad clusters to go to
  // the same track an output collection name a verbosity controller

  if (verbose_) {
    ldmx_log(debug)
        << "TrigScintTrackProducer: produce() starts! Event number: "
        << event.getEventHeader().getEventNumber();
  }
  if (!event.exists(seeding_collection_, passName_)) {
    ldmx_log(info) << "No collection called " << seeding_collection_
                   << "; skipping event";
    //                   << "; still, not skipping event";                                                                                                                       
    return;
  }

  const auto seeds{event.getCollection<ldmx::TrigScintCluster>(
      seeding_collection_, passName_)};
  uint numSeeds = seeds.size();

  if (verbose_) {
    ldmx_log(debug) << "Got track seeding cluster collection "
                    << seeding_collection_ << " with " << numSeeds
                    << " entries ";
  }

  if (!event.exists(input_collections_.at(0))) {
    ldmx_log(info) << "No collection called " << input_collections_.at(0)
                   << "; skipping event";
	//                   << "; still, not skipping event";
	return;
  }
  const auto clusters_pad1{event.getCollection<ldmx::TrigScintCluster>(
      input_collections_.at(0), passName_)};

  if (!event.exists(input_collections_.at(1))) {
    ldmx_log(info) << "No collection called " << input_collections_.at(1)
                   << ";  skipping event";
	//                   << "; still, not skipping event";
	return;
  }

  const auto clusters_pad2{event.getCollection<ldmx::TrigScintCluster>(
      input_collections_.at(1), passName_)};

  if (verbose_) {
    ldmx_log(debug) << "Got the other two pad collections:"
                    << input_collections_.at(0) << " with "
                    << clusters_pad1.size() << " entries, and "
                    << input_collections_.at(1) << " with "
                    << clusters_pad2.size() << " entries.";
  }

  std::vector<ldmx::TrigScintTrack> cleanedTracks;
  std::vector<ldmx::TrigScintTrack> cleanedTracksY;
  std::vector<ldmx::TrigScintTrack> cleanedTracksX;

  // loop over the clusters in the seeding pad collection, if there are clusters
  // in all pads
  bool skipDn = false;
  if (numSeeds && clusters_pad1.size() ) {
	// could check this explicitly here: and then just get out of all checks on the dn pad immediately 
	//	if (! clusters_pad2.size())
	//skipDn = true ;
    for (const auto &seed : seeds) {
      // for each seed, search through the other two pads to match all clusters
      // with centroids within tolerance to tracks
      float centroid = seed.getCentroid();

      std::vector<ldmx::TrigScintTrack> trackCandidates;

      if (verbose_ > 1) {
        ldmx_log(debug) << "Got seed with centroid " << centroid;
      }

      // reset for each seed
      bool madeTrack = false;

      for (const auto &cluster1 : clusters_pad1) {
        if (verbose_ > 1) {
          ldmx_log(debug) << "\tGot pad1 cluster with centroid "
                          << cluster1.getCentroid();
        }
        if (fabs(cluster1.getCentroid() - centroid) <
            maxDelta_ ||
			( centroid >= vertBarStartIdx_    //then in vertical bars
			  && seed.getCentroidX() == cluster1.getCentroidX() )	) {
			//use geometry y overlap scheme to see if this is really a match in x
		  // should be done in a map

		  if ( centroid >= vertBarStartIdx_  &&
			   seed.getCentroidY() < cluster1.getCentroidY() ) {
			//impossible combination
			if (verbose_ > 1) {
                  ldmx_log(debug) << "\tSkipping impossible x cluster combination with y flags (tag up) ("
                                  << seed.getCentroidY() << " "
                                  << cluster1.getCentroidY()  << ")";
			}
			continue;
		  }
		  
		  // else: first (possible) match! loop through next pad too
		  
          if (verbose_ > 1) {
            ldmx_log(debug) << "\t\tIt is close enough!. Check pad2";
          }

		  
		  //try making third pad clusters an optional part of track

		  std::vector<ldmx::TrigScintCluster> clusterVec = {seed, cluster1};

		  bool hasMatchDn = false;
		  
          for (const auto &cluster2 : clusters_pad2) {
            if (verbose_ > 1) {
              ldmx_log(debug) << "\tGot pad2 cluster with centroid "
                              << cluster2.getCentroid();
            }
			
            if (fabs(cluster2.getCentroid() - centroid) <
                maxDelta_ ||
				( centroid >= vertBarStartIdx_    //then in vertical bars
				  && seed.getCentroidX() == cluster2.getCentroidX() )  ) {
			  //use geometry y overlap scheme to see if this is really a match in x
			  
			  if (centroid >= vertBarStartIdx_ &&
				  (seed.getCentroidY() < cluster2.getCentroidY() ||
				   cluster1.getCentroidY() > cluster2.getCentroidY()) )
				{// impossible
				  if (verbose_ > 1) {
					ldmx_log(debug) << "\tSkipping impossible x cluster combination with y flags (tag up dn) ("
									<< seed.getCentroidY() << " "
									<< cluster1.getCentroidY()  << " "
									<< cluster2.getCentroidY()  << ")";
				  }
				  continue;
				}
			  
			  // first match! loop through next pad too
			  
			  if (verbose_ > 1) {
				ldmx_log(debug) << "\t\tIt is close enough!. Make a track";
			  }
			  
			  // only make this vector now! this ensures against hanging
			  // clusters with indices from earlier in the loop
			  std::vector<ldmx::TrigScintCluster> threeClusterVec = {seed, cluster1,
																	 cluster2};
			  
              /*
              // here we could break if we didn't want to allow all possible
              combinations madeTrack=true; break; //we're done with this
              iteration once there's a track made
              */
			  // make a track
			  ldmx::TrigScintTrack track = makeTrack(threeClusterVec);
			  trackCandidates.push_back(track);
			  hasMatchDn=true;
            }  // if match in pad2
          }    // over clusters in pad2
		  // if there was no match to this in pad 2, make a track with just these two clusters
		  if ( ! hasMatchDn &&  skipLast_ ) { //we allow skipping last pad if needed
			ldmx::TrigScintTrack track = makeTrack(clusterVec);
			trackCandidates.push_back(track);
		  }

	  } // if possible (x,)y match in pad1
		/*
        //same here
        if (madeTrack)
        break;
        */
	  
	}  // over clusters in pad1

      // continue to next seed if 0 track candidates
      if (trackCandidates.size() == 0) continue;

      int keepIdx = 0;
      float minResidual = 1000;  // some large number

      // no need to choose between only one candidate track
      if (trackCandidates.size() > 1) {
        // now for each seed, pick only the track with the smallest residual.

        if (verbose_) {
          ldmx_log(debug) << "Got " << trackCandidates.size()
                          << " tracks to check.";
        }

        for (uint idx = 0; idx < trackCandidates.size(); idx++) {
          if ((trackCandidates.at(idx)).getResidual() < minResidual) {
            keepIdx = (int)idx;
            minResidual =
                (trackCandidates.at(idx)).getResidual();  // update minimum

            if (verbose_ > 1) {
              ldmx_log(debug)
                  << "Track at index " << idx
                  << " has smallest residual so far: " << minResidual;
            }

          }  // finding min residual
        }    // over track candidates
      }      // if more than 1 to choose from

      // store the track at keepIdx, if there was one we made it this far and
      // keepIdx is 0 or has been updated to the smallest residual track idx
      //	if (keepIdx >= 0) {
      tracks_.push_back(trackCandidates.at(keepIdx));
      if (verbose_) {
        ldmx_log(debug) << "Kept track at index " << keepIdx;
        (trackCandidates.at(keepIdx)).Print();
      }
      //}
    }  // over seeds

    // done here if there were no tracks found
    if (tracks_.size() == 0) {
      if (verbose_) {
        ldmx_log(debug) << "No tracks found!";
      }
	  return;
    }
    // now, if there are multiple seeds sharing the same downstream hits, this
    // should also be remedied with a selection on min residual.

    // The logic of this loop kind of assumes I can remove tracks immediately --
    // that way I can do pairwise checks between more tracks within a single
    // loop. But for now I haven't figured out how to erase elements in a fool
    // proof way. So I iterate over a vector...

    std::vector keepIndices(tracks_.size(), 1);
    if (verbose_ > 1)
      ldmx_log(debug) << "vector of indices to keep has size "
                      << keepIndices.size();
	
    for (uint idx = tracks_.size() - 1; idx > 0; idx--) {
      // since we start in one end, we only have to check matches in one
      // direction
      ldmx::TrigScintTrack track = tracks_.at(idx);
      for (int idxComp = idx - 1; idxComp >= 0; idxComp--) {
        if (verbose_ > 1)
          ldmx_log(debug) << "In track disambiguation loop, idx points at "
                          << idx << " and prev idx points at " << idxComp;

        ldmx::TrigScintTrack nextTrack = tracks_.at(idxComp);

        // no need to start pulling constituents from tracks that are
        // ridiculously far apart
        if (fabs(track.getCentroid() - nextTrack.getCentroid() <
                 3 * maxDelta_)) {
          std::vector<ldmx::TrigScintCluster> consts_1 =
              track.getConstituents();
          std::vector<ldmx::TrigScintCluster> consts_2 =
              nextTrack.getConstituents();
          if (verbose_ > 1)
            ldmx_log(debug)
                << "In track disambiguation loop, got the two tracks, "
                   "with nConstituents "
                << consts_1.size() << " and " << consts_2.size()
                << ", respectively. ";
          // let's do "if either cluster is shared" right now... but could also
          // have it settable to use a stricter cut: an AND
          if (consts_1[1].getCentroid() == consts_2[1].getCentroid() ||
			  (consts_1.size() > 2 && consts_2.size() > 2 &&
			   consts_1[2].getCentroid() ==
			   consts_2[2].getCentroid())) {  // we have overlap downstream of the
            // seeding pad. probably, one cluster
            // in seeding pad is noise

            if (verbose_ > 1) {
              ldmx_log(debug) << "Found overlap! Tracks at index " << idx
                              << " and " << idxComp;
              (tracks_.at(idx)).Print();
              (tracks_.at(idxComp)).Print();
            }

            if ((tracks_.at(idx)).getResidual() <
                (tracks_.at(idxComp)).getResidual()) {
              // next track (lower index) is a worse choice, remove its flag for
              // keeping
              keepIndices.at(idxComp) = 0;
            } else  // prefer next track over current. remove current track's
                    // keep
              // flag
              keepIndices.at(idx) = 0;
            /*}
                  else {
                  tracks_.erase(itNext);
                  //        removeIdx.push_back(idx+1);
                  // we might see the same index two times in the loop in this
               case, if there are three seeds sharing the same clusters
               downstream.
                  // then the third only gets removed if it's even worse than
               the second.
                  // one could deal with this with an extra overlap check. not
               sure we will be in this situation any time soon though.
                  }*/
          }  // over matching/overlapping tracks
        }    // over tracks close enough to share constituents
      }      // over constructed tracks at other indices, to match
    }        // over constructed tracks
	
    for (uint idx = 0; idx < tracks_.size(); idx++) {
      if (verbose_ > 1) {
        ldmx_log(debug) << "keep flag for idx " << idx << " is "
                        << keepIndices.at(idx);
      }
      if (keepIndices.at(idx)) {  // this hasn't been flagged for removal
		
        cleanedTracks.push_back(tracks_.at(idx));
		
        if (verbose_) {
          ldmx_log(debug) << "After cleaning, keeping track at index " << idx
                          << ": Centroid = " << (tracks_.at(idx)).getCentroid()
                          << "; CentroidX = " << (tracks_.at(idx)).getCentroidX()
                          << "; CentroidY = " << (tracks_.at(idx)).getCentroidY()
                          << "; track PE = " << (tracks_.at(idx)).getPE();
		  //          (tracks_.at(idx)).Print();
        }
      }  // if index flagged for keeping
    }    // over all (uniquely seeded) tracks in the event
    /*
      if (verbose_ ) {
      for (uint idx=0; idx < tracks_.size(); idx++){
      ldmx_log(debug)<< "Keeping track at index " << idx << ":";
      (tracks_.at(idx)).Print();
      }
      }
    */
	
	
	
	if (verbose_) {
	  ldmx_log(debug) << "Running track x,y matching ";
	}
	
	if (cleanedTracks.size() > 0 )
	  {
		matchXYTracks( cleanedTracks );
		std::vector<ldmx::TrigScintTrack> matchedTracks = cleanedTracks; //don't know why this copying needs to happen but it does
		//	std::vector<ldmx::TrigScintTrack>  matchXYTracks( cleanedTracks );
		//		std::vector<ldmx::TrigScintTrack> matchedTracks = matchXYTracks( cleanedTracks );
		for (auto trk : matchedTracks ) {
		  /*	for (uint idx = 0; idx < tracks_.size(); idx++) {
				if (verbose_ > 1) {
				ldmx_log(debug) << "keep flag for idx " << idx << " is "
						<< keepIndices.at(idx);
						}
						if (keepIndices.at(idx)) {  // this hasn't been flagged for removal   
		//check if channel nb is above that of horizontal bars
		if (tracks_.at(idx).getCentroid() >= vertBarStartIdx_) 
		  */
		  if (trk.getCentroid() >= vertBarStartIdx_) 
		cleanedTracksX.push_back(trk); //acks_.at(idx));
		  else 
			cleanedTracksY.push_back(trk);//acks_.at(idx));
		  //		cleanedTracksY.push_back(trk);
		  if (verbose_ > 1) {
		float centr = trk.getCentroid(); //tracks_.at(idx).getCentroid(); //
		std::string collStr = centr >= vertBarStartIdx_ ? "X" : "Y" ;
		collStr=output_collection_+collStr;
		ldmx_log(debug) << "saving track with centroid " << centr << " to output track collection " << collStr ;
		  }
		  // }
		}
	  }
	
	
  }  // if there are clusters in all pads
  else if (verbose_) {
    ldmx_log(info) << "Not all pads had clusters; (maybe) skipping tracking attempt";
  }

  
  if (verbose_) {
    ldmx_log(debug) << "Done with tracking step. ";
  }

  
  event.add(output_collection_, cleanedTracks);
  //  event.add(output_collection_, matchedTracks);

  event.add(output_collection_+"Y", cleanedTracksY);
  event.add(output_collection_+"X", cleanedTracksX);

  
  tracks_.resize(0);

  return;
}

ldmx::TrigScintTrack TrigScintTrackProducer::makeTrack(
    std::vector<ldmx::TrigScintCluster> clusters) {
  // for now let's keep a straight, unweighted centroid
  // consider the possibility that at least one cluster has a centroid
  // identically == 0. then we need to shift them by 1 if we want to do energy
  // weighted track centroid later. but no need now
  ldmx::TrigScintTrack tr;
  float centroid = 0;
  float centroidX = 0;
  float centroidY = 0;
  float beamEfrac = 0;
  float pe = 0;
  for (uint i = 0; i < clusters.size(); i++) {
    centroid += (clusters.at(i)).getCentroid();
    centroidX += (clusters.at(i)).getCentroidX();
    centroidY += (clusters.at(i)).getCentroidY();
    tr.addConstituent(clusters.at(i));
    beamEfrac += (clusters.at(i)).getBeamEfrac();
    pe += (clusters.at(i)).getPE();
  }
  centroid /= clusters.size();
  centroidX /= clusters.size();
  if (centroid >= vertBarStartIdx_) {
	  if (verbose_) {
		ldmx_log(debug) << " --  In makeTrack made vertical bar track with centroid  "
						<< centroid << " and y flag sum " << centroidY ;
		//try commenting this to check if that helps with an out-of-bounds problem
		//                    << " from clusters with y centroids";
		//for (uint i = 0; i < clusters.size(); i++)
		// ldmx_log(debug) << "\tpad " << i << ": centroidY "
		//				  << (clusters.at(i)).getCentroidY();
	  }
	  //then the sum of centroid y is 0, 2, 4 or 6
	  // we have 4 divisions, so, the center of it should be divNb/8
	  // (or rather, that's where channel nBars/8 begins)
	  // and then a factor 2 for the zig-zag pattern
	  centroidY = (centroidY+1)*2*nBarsY_/8.;
	  // TODO: here we could instead just use quadrant indices 0-3 by dividing by 2
	  // but that would mean that in the raw, x and y track centroidY would mean different things
	  if (verbose_) 
        ldmx_log(debug) << " --  new centroidY = " << centroidY ;
  }
  else
	centroidY /= clusters.size();

  beamEfrac /= clusters.size();
  pe /= clusters.size();

  float residual = 0;
  for (uint i = 0; i < clusters.size(); i++)
    residual += ((clusters.at(i)).getCentroid() - centroid) *
                ((clusters.at(i)).getCentroid() - centroid);
  residual = sqrt((float)(residual / clusters.size()));

  tr.setCentroid(centroid);
  tr.setCentroidX(centroidX);
  tr.setCentroidY(centroidY);
  tr.setResidual(residual);
  tr.setBeamEfrac(beamEfrac);
  tr.setPE(pe);

  if (verbose_) {
    ldmx_log(debug) << " --  In makeTrack made track with centroid  "
                    << centroid << " and residual " << residual
					<< " and pe " << pe
                    << " from clusters with centroids";
    for (uint i = 0; i < clusters.size(); i++)
      ldmx_log(debug) << "\tpad " << i << ": centroid "
                      << (clusters.at(i)).getCentroid();
  }

  return tr;
}



  //std::vector<ldmx::TrigScintTrack>  TrigScintTrackProducer::matchXYTracks(
  void TrigScintTrackProducer::matchXYTracks(
    std::vector<ldmx::TrigScintTrack> &tracks) {

  //map quadrant nb to track (can be multiple per quadrant)
  std::multimap <int, int > yIdxQuadMap; //key = quad, val = track index in collection
	std::multimap <int, int > xIdxQuadMap;

  std::multimap <int, ldmx::TrigScintTrack > yQuadMap;
  std::multimap <int, ldmx::TrigScintTrack > xQuadMap;
  //map track in quadrant back to index in entire track collection
  //used for updating collection track variables 
  std::map <ldmx::TrigScintTrack, int > yTrackMap;
  std::map <ldmx::TrigScintTrack, int > xTrackMap;
  
  uint trkIdx = -1;
  for ( auto trk : tracks ) {
	trkIdx++;
	//1. get the y bar tracks with centroidX = -1
	if ( trk.getCentroidX() == -1) {
	  if (verbose_) 
		ldmx_log(debug) << " --  In matchXYTracks found y track at " << trk.getCentroidY() << "; mapping to quad " << (int)trk.getCentroidY()/8 << " with trk index " << trkIdx ;
	  //2. order them... or map them to quadrants. note that there are 2 layers so 2*nBarsY_/4 channels per quadrant  
	  yQuadMap.insert(std::make_pair( (int)(trk.getCentroidY()/8), trk)); 
	  yTrackMap[ trk ] = trkIdx;
	  yIdxQuadMap.insert(std::make_pair( (int)(trk.getCentroidY()/8), trkIdx)); 
	  
	}
	else {  //3. get the remaining tracks (from vertical bars) and map them (back) to (middle of) quadrants 
	  xQuadMap.insert(std::make_pair( (int)(trk.getCentroidY()/8), trk));
	  xTrackMap[ trk ] = trkIdx;
	  xIdxQuadMap.insert(std::make_pair( (int)(trk.getCentroidY()/8), trkIdx)); 
	  if (verbose_) 
		ldmx_log(debug) << " --  In matchXYTracks found x track at (x,y) = (" << trk.getCentroidX() << ", " << trk.getCentroidY() << "); mapping to quad " << (int)trk.getCentroidY()/8 << " with trk index " << trkIdx ;
	}
  }
  

  // 4a1) here use the geometry? if we can assume perfect alignment we can take width and nBars and take nBars/2 as origin
  // --- now do the matching ---

  //if there is no useful matching to be done: these are the pad width wide numbers 
  float x0 = 0;
  float sx0 = fabs(xStart_); // this should be half the pad... could also set it to full beam spot width
  float sy0 = fabs(yStart_)/4.; // yStart_ is half the pad, so this should be half a quadrant
  
  //assume at least one y track. will have to figure out if there is ever a reason to use an isolated x track in its place.
  for ( auto yitr = yQuadMap.begin(); yitr != yQuadMap.end(); ++yitr) {
	int nYinQuad =	yQuadMap.count( (*yitr).first );
	int nXinQuad =	xQuadMap.count( (*yitr).first );
	float y, sy, x, x1, x2, sx1, sx2, y1, y2, sy1, sy2;
	//quad midpoint: 
	float y0 = (*yitr).first*8 + sy0;
	float sx  = 1./2*xConvFactor_; //rely on x precision being one single bar width; always used unless x is undeterminable 

	//check all x first
	//do the easiest first:
	if (nXinQuad == 0 ) { // then there's no hope of setting a better x here
	  // just use the beam spot width... and center of pad 
	  x = x0;
	  sx = sx0;
	  if (verbose_)
		ldmx_log(debug) << "\t\t\t no x info in quad " << (*yitr).first << "; will set x to middle of pad, pad half-width as precision: set (x, sx)=(" << x << ", " << sx << ")";
	} //0 x tracks in quadrant
	else if (nXinQuad == 1 ) { 	//slightly harder: 1 x track -- might be easy if it's just one y track; if several, need to think about overlaps. but in overlap case, just revert to setting x0 and sx0, when we know  
	  auto xitr = xQuadMap.find( (*yitr).first );
	  x = ((*xitr).second).getCentroidX() * xConvFactor_ + xStart_;

	  if (verbose_)
		ldmx_log(debug) << "\t\t\t 1 x in quad " << (*yitr).first << ", getting (x, sx)=(" << x << ", " << sx << ")";
	  }// 1 x track in quadrant 
	else if (nXinQuad == 2) {	// finally if we have two tracks, get x1 and x2 and decide later how to use them
	  // don't think we want to experiment with discerning three overlapping tracks, so not >= 2
	  //		  continue; //debugging: skip for now -- didn't help
	  auto xitr1 = xQuadMap.lower_bound( (*yitr).first );
	  auto xitr2 = xQuadMap.upper_bound( (*yitr).first );
	  xitr2--; //upper_bound points to next element

	  if (xitr1 != xitr2) {// should be true already but...   
		x1  = ((*xitr1).second).getCentroidX() * xConvFactor_ + xStart_;
		x2  = ((*xitr2).second).getCentroidX() * xConvFactor_ + xStart_;
		sx1 = xConvFactor_/2.;  //1 bar width 
		sx2 = sx1; 
		x = (x1+x2)/2.;
		sx = fabs(x1-x2)/2*xConvFactor_; //rely on x precision being one single pad width
		if (verbose_)
		  ldmx_log(debug) << "\t\t -- 2 x in quad: setting y track x coordinate to midpoint";
	  }
	}// if 2 x tracks in quad 

	
	//ok! over y:
	// can skip 0 y case by construction 
	if (nYinQuad == 1 ){  // we can already now tell what the y coordinate and its precision is	  
      y = ((*yitr).second).getCentroidY() * yConvFactor_ + yStart_;
	  sy = ((*yitr).second).getResidual() * yConvFactor_;
	  if (sy == 0 )
		sy = 1./2*yConvFactor_; //if all clusters lined up, assign precision of 1 bar width                                                                                       

	  if (nXinQuad <= 1 ) {
		//4. every quadrant which just has one of each --> done ;      
		// b) set the sx, sy of the x track now, using the residuals from the other
		// b1) special case: no x tracks; then x, sx have been set above 
		auto xidx = xIdxQuadMap.find( (*yitr).first );
		auto yidx = yIdxQuadMap.find( (*yitr).first );
		tracks.at( (*xidx).second).setPosition(x,y);
		tracks.at( (*xidx).second).setSigmaXY(sx, sy);
		tracks.at( (*yidx).second).setPosition(x,y);
		tracks.at( (*yidx).second).setSigmaXY(sx, sy);
		if (verbose_)
		  ldmx_log(debug) << "\t\t\t in quad " << (*yitr).first << ", set (x, y) = (" << x << ", " << y << ") and (sx, sy) = "<< sx << ", " << sy << ")";
		continue;
	  }
	}// 1 y, 0 or 1 x track in quadrant 
	
	if (verbose_)
	  ldmx_log(debug) << "\t\t in quad " << (*yitr).first << ", not single x,y tracks: " << nXinQuad << " of x and "<< nYinQuad << " of y";

	if (nYinQuad == 2) {// let's start here and see if we can do >= 2 later
	  //here one could do sth to avoid checking the other y track again in the outermost loop over y 
	  auto yitr1 = yQuadMap.lower_bound( (*yitr).first );
	  auto yitr2 = yQuadMap.upper_bound( (*yitr).first );
	  yitr2--; //back up once 
	  y1  = ((*yitr1).second).getCentroidY() * yConvFactor_ + yStart_;
	  y2  = ((*yitr2).second).getCentroidY() * yConvFactor_ + yStart_;
	  sy1 = ((*yitr1).second).getResidual()*yConvFactor_;
	  sy2 = ((*yitr2).second).getResidual()*yConvFactor_;
	  y = (y1+y2)/2.;
	  sy = fabs(y1-y2)/2*yConvFactor_;
	  if (verbose_)
		ldmx_log(debug) << "\t\t -- 2 y in quad: setting x track y coordinate to midpoint";
	} //2y in quad 

	if (nYinQuad == 1 && nXinQuad == 2) {// don't think we want to experiment with discerning three overlapping tracks, so not >= 2

	  //first: set the y track coordinates to x  = the mid of x tracks, y = y of y track 
	  auto yidx = yIdxQuadMap.find( (*yitr).first );
	  tracks.at( (*yidx).second).setPosition(x,y);
	  tracks.at( (*yidx).second).setSigmaXY(sx, sy);			  

	  int minOverlapPE_ = 250;
	  if (((*yitr).second).getPE() < minOverlapPE_ ) {
		// can't tell, really, that either of these belong to the y track. so.
		// let them keep their own x coordinate but set y to quadrant midpoint, with uncertainty +/- half quadrant width (1/8 of pad height)
		y = y0;
		sy = sy0;
		if (verbose_)
		  ldmx_log(debug) << "\t\t -- Can't tell which x track should be matched to single y track. Setting both x track coordinates to y quadrant value:" ;
	  }// if can't assume overlap 
	  else 
		if (verbose_)
		  ldmx_log(debug) << "\t\t -- Found large PE count (" << ((*yitr).second).getPE() << " > " << minOverlapPE_
						  << "), suggesting overlap! Setting both x track coordinates to y track value:" ;
		
	  // consider making two x tracks out if this one, and, anyway have to set their average as the y track x cocordinate
	  //EXPERIMENTAL : apply only to x tracks, which can be disregarded for electron counting
	  if (verbose_) ldmx_log(debug) << "\t\t --  (x1, x2, y) = ("
									<< x1 << ", " << x2 << ", " << y << ") and (sx1, sx2, sy) = "
									<< sx1 << ", " << sx2 << ", "<< sy << ")";
		
	  //now set x track coordinates according to overlap check result 
	  auto xidx1 = xIdxQuadMap.lower_bound( (*yitr).first );
	  auto xidx2 = xIdxQuadMap.upper_bound( (*yitr).first );
	  xidx2--; //upper_bound points to (last+1) element
	  tracks.at( (*xidx1).second).setPosition(x1,y);
	  tracks.at( (*xidx1).second).setSigmaXY(sx1, sy);			  
	  tracks.at( (*xidx2).second).setPosition(x2,y);
	  tracks.at( (*xidx2).second).setSigmaXY(sx2, sy);
		
	}// 1 y, 2 x tracks in the quadrant 
	else if (nYinQuad == 2 && nXinQuad == 1) {
	  // 5b) if there are more y than x: could be an overlap
                                                                                                    
	  //first: set the x track coordinates to x = x of x track, y = the mid of y tracks
	  auto xidx  = xIdxQuadMap.find( (*yitr).first );
	  tracks.at( (*xidx).second).setPosition(x,y);
	  tracks.at( (*xidx).second).setSigmaXY(sx, sy);
		
	  auto xitr  = xQuadMap.lower_bound( (*yitr).first );
	  int minOverlapPE_ = 300;
	  if (((*xitr).second).getPE() < minOverlapPE_ ) {
		if (verbose_)
		  ldmx_log(debug) << "\t\t just 1 x track with not-unusual PE in the quad -- can't match; setting mid-point values for x ";
		x = x0;
		sx = sx0 ;
	  }//if can't assume overlap
	  else {
		// consider making two x tracks out if this one, and, anyway have to set their average as the y track x cocordinate      
		//EXPERIMENTAL : apply only to x tracks, which can be disregarded for electron counting                                                                                
		if (verbose_)
		  ldmx_log(debug) << "\t\t -- Found large PE count (" << ((*xitr).second).getPE() << " > " << minOverlapPE_
						  << ") in x track, suggesting overlap! Setting both y track coordinates to x track value:" ;
	  }// if can assume overlap
	  if (verbose_)
		ldmx_log(debug) << "\t\t --  (x, y1, y2) = ("
						<< x << ", " << y1 << ", " << y2 << ") and (sx, sy1, sy2) = "
						<< sx << ", " << sy1 << ", "<< sy2 << ")";
		
	  auto yidx1 = yIdxQuadMap.lower_bound( (*yitr).first );
	  auto yidx2 = yIdxQuadMap.upper_bound( (*yitr).first );
	  yidx2--; //upper_bound points to next element
	  tracks.at( (*yidx1).second).setPosition(x,y1);
	  tracks.at( (*yidx1).second).setSigmaXY(sx, sy1);
	  tracks.at( (*yidx2).second).setPosition(x,y2);
	  tracks.at( (*yidx2).second).setSigmaXY(sx, sy2);

	}// 2 y and 1 x track in quad 
	else if ( nYinQuad == 2 && nXinQuad == 2 ) {
	  //MIDPONTS ALL OVER!
	  auto xidx1 = xIdxQuadMap.lower_bound( (*yitr).first );
	  auto xidx2 = xIdxQuadMap.upper_bound( (*yitr).first );
	  xidx2--;
	  auto yidx1 = yIdxQuadMap.lower_bound( (*yitr).first );
	  auto yidx2 = yIdxQuadMap.upper_bound( (*yitr).first );
	  yidx2--;

	  if ( yIdxQuadMap.find( (*yitr).first )== yIdxQuadMap.end() )
		ldmx_log(error) << "The two y tracks in the same quadrant at " << (*yitr).first << " appear to not be found in the y track map! investigate. Note that yidx1.first = " << (*yidx1).first << " and  yidx2.first = " << (*yidx2).first ;
	  else {
		tracks.at( (*xidx1).second).setPosition(x1,y);
		tracks.at( (*xidx1).second).setSigmaXY(sx1, sy);
		tracks.at( (*xidx2).second).setPosition(x2,y);
		tracks.at( (*xidx2).second).setSigmaXY(sx2, sy);
		  
		tracks.at( (*yidx1).second).setPosition(x,y1);
		tracks.at( (*yidx1).second).setSigmaXY(sx, sy1);
		tracks.at( (*yidx2).second).setPosition(x,y2);
		tracks.at( (*yidx2).second).setSigmaXY(sx, sy2);

		if (verbose_)
		  ldmx_log(debug) << "\t\t -- in a 2 x 2 situaiton; midpoint y: " << y << " for both x tracks, midpoint x: " << x << " for both y tracks";
	  }
	} // if 2 y, 2 x tracks 
	
	if ( nXinQuad > 2) {
	  if (verbose_)
		ldmx_log(debug) << "\t\t -*-*-*- more than 2 x tracks in the same quad -- nothing done about the x,y coordinates in this situation -- implement if needed!!" ;
	}
	if ( nYinQuad > 2) {
	  if (verbose_)
		ldmx_log(debug) << "\t\t -*-*-*- more than 2 y tracks in the same quad -- nothing done about the x,y coordinates in this situation -- implement if needed!!" ;
	}
	
  }// over y tracks 
  
  yQuadMap.clear();
  xQuadMap.clear();


  //  return tracks;
  
}
  
void TrigScintTrackProducer::onFileOpen() {
  ldmx_log(debug) << "Opening file!";

  return;
}

void TrigScintTrackProducer::onFileClose() {
  ldmx_log(debug) << "Closing file!";

  return;
}

void TrigScintTrackProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigScintTrackProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintTrackProducer);
