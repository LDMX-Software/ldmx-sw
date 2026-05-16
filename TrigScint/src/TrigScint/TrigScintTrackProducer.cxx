#include "TrigScint/TrigScintTrackProducer.h"

#include <iterator>  // std::next
#include <map>

namespace trigscint {

void TrigScintTrackProducer::configure(framework::config::Parameters &ps) {
  max_delta_ = ps.get<double>(
      "delta_max");  // max distance to consider adding in a cluster to track   
  max_delta_vert= ps.get<double>("delta_vert_max"); //max distance between pad 1/2 and 3 along the x axis 
  //to consider make a track using the vertical bars 
  seeding_collection_ = ps.get<std::string>(
      "seeding_collection");  // probably tagger pad, "TriggerPadTagClusters"
  input_collections_ = ps.get<std::vector<std::string>>(
      "further_input_collections");  // {"TriggerPadUpClusters" ,
                                     // "TriggerPadDnClusters" }
  output_collection_ = ps.get<std::string>("output_collection");
  pass_name_ = ps.get<std::string>("input_pass_name");
  verbose_ = ps.get<int>("verbosity");
  vert_bar_start_idx_ = ps.get<int>("vertical_bar_start_index");
  n_bars_y_ = ps.get<int>("number_horizontal_bars");
  bar_width_y_ = ps.get<double>("horizontal_bar_width");
  bar_gap_y_ = ps.get<double>("horizontal_bar_gap");
  n_bars_x_ = ps.get<int>("number_vertical_bars");
  bar_width_x_ = ps.get<double>("vertical_bar_width");
  bar_gap_x_ = ps.get<double>("vertical_bar_gap");
  skip_last_ = ps.get<bool>("allow_skip_last_collection");
  bar_length_y_=ps.get<double>("horizontal_bar_length"); //bar lenght of the horizontal bars

  // TO DO: allow any number of input collections

  if (verbose_) {
    ldmx_log(info) << "In TrigScintTrackProducer: configure done!" << std::endl;
    ldmx_log(info) << "Got parameters: \nSeeding:   " << seeding_collection_
                   << "\nTolerance: " << max_delta_
                   << "\nInput:     " << input_collections_.at(0) << " and "
                   << input_collections_.at(1)
                   << "\nInput pass name:     " << pass_name_
                   << "\nAllow tracks with no hit in last collection:     "
                   << skip_last_
                   << "\nVertical bar start index:     " << vert_bar_start_idx_
                   << "\nNumber of horizontal bars:     " << n_bars_y_
                   << "\nHorizontal bar width:     " << bar_width_y_
                   << "\nHorizontal bar gap:     " << bar_gap_y_
                   << "\nNumber of vertical bars:     " << n_bars_x_
                   << "\nVertical bar width:     " << bar_width_x_
                   << "\nVertical bar gap:     " << bar_gap_x_
                   << "\nOutput:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }
  // each bar only goes half this distance up (overlap/zig-zag)
  y_conv_factor_ = (bar_width_y_ + bar_gap_y_) / 2.;
  // half height of pad
  y_start_ = -(n_bars_y_ * (bar_width_y_ + bar_gap_y_) - bar_gap_y_) / 2.;
  // each bar goes entire distance sideways (no overlap)
  x_conv_factor_ = bar_width_x_ + bar_gap_x_;
  // half width of pad
  x_start_ = -(n_bars_x_ * (bar_width_x_ + bar_gap_x_) - bar_gap_x_) / 2.;

  return;
}

void TrigScintTrackProducer::produce(framework::Event &event) {
  // parameters.
  // one pad cluster collection to use as seed
  // a vector with the other two
  // a maximum distance between seed centroid and other pad clusters
  //   allowed to belong to the same track
  // an output collection name a verbosity controller

  if (verbose_) {
    ldmx_log(debug)
        << "TrigScintTrackProducer: produce() starts! Event number: "
        << event.getEventHeader().getEventNumber();
  }
  if (!event.exists(seeding_collection_, pass_name_)) {
    ldmx_log(info) << "No collection called " << seeding_collection_
                   << "; skipping event";
    //                   << "; still, not skipping event";
    return;
  }

  if (!event.exists(seeding_collection_, pass_name_)) {
    ldmx_log(info) << "No collection called " << seeding_collection_
                   << "; skipping event";
    return;
  }
  const auto seeds{event.getCollection<ldmx::TrigScintCluster>(
      seeding_collection_, pass_name_)};
  uint num_seeds = seeds.size();

  if (verbose_) {
    ldmx_log(debug) << "Got track seeding cluster collection "
                    << seeding_collection_ << " with " << num_seeds
                    << " entries ";
  }

  if (!event.exists(input_collections_.at(0), pass_name_)) {
    ldmx_log(info) << "No collection called " << input_collections_.at(0)
                   << "; skipping event";
    //                   << "; still, not skipping event";

    return;
  }
  const auto clusters_pad1{event.getCollection<ldmx::TrigScintCluster>(
      input_collections_.at(0), pass_name_)};

  if (!event.exists(input_collections_.at(1), pass_name_)) {
    ldmx_log(info) << "No collection called "
                   << input_collections_.at(1)
                   //                   << "; still, not skipping event";
                   << "; skipping event";
    std::vector<ldmx::TrigScintTrack> empty{};
    event.add(output_collection_, empty);
    return;
  }

  const auto clusters_pad2{event.getCollection<ldmx::TrigScintCluster>(
      input_collections_.at(1), pass_name_)};

  if (verbose_) {
    ldmx_log(debug) << "Got the other two pad collections:"
                    << input_collections_.at(0) << " with "
                    << clusters_pad1.size() << " entries, and "
                    << input_collections_.at(1) << " with "
                    << clusters_pad2.size() << " entries.";
  }
        ////////////////////////// Ricardo
  if (verbose_ > 1) {

    ldmx_log(debug) << "max_delta = " << max_delta_;
    
    
    auto print_cluster = [&](const std::string& name,
                            const ldmx::TrigScintCluster& cl) {
      ldmx_log(debug) << name
                      << " c=" << cl.getCentroid()
                      << " cx=" << cl.getCentroidX()
                      << " cy=" << cl.getCentroidY()
                      << " type=" << (cl.getCentroid() >= vert_bar_start_idx_ ? "VERT" : "HORIZ");
    };

    ldmx_log(debug) << "=== SEEDS ===";
    for (const auto& cl : seeds) print_cluster("seed", cl);

    ldmx_log(debug) << "=== PAD1 ===";
    for (const auto& cl : clusters_pad1) print_cluster("pad2", cl);

    ldmx_log(debug) << "=== PAD2 ===";
    for (const auto& cl : clusters_pad2) print_cluster("pad3", cl);
  }
                //////////////////////////////////////
  std::vector<ldmx::TrigScintTrack> cleaned_tracks;
  std::vector<ldmx::TrigScintTrack> cleaned_tracks_y;
  std::vector<ldmx::TrigScintTrack> cleaned_tracks_x;

  // loop over the clusters in the seeding pad collection, if there are clusters
  // in all pads
  // bool skipDn = false;
  if (num_seeds && clusters_pad1.size()) {
    // could check this explicitly here: and then just get out of all checks on
    // the dn pad immediately
    //	if (! clusters_pad2.size())
    // skipDn = true ;
    for (const auto &seed : seeds) {
      // for each seed, search through the other two pads to match all clusters
      // with centroids within tolerance to tracks
      float centroid = seed.getCentroid();

      std::vector<ldmx::TrigScintTrack> track_candidates;

      if (verbose_ > 1) {
        ldmx_log(debug) << "Got seed with centroid " << centroid;
      }

      // reset for each seed
      // bool madeTrack = false;

      for (const auto &cluster1 : clusters_pad1) {
        if (verbose_ > 1) {
          ldmx_log(debug) << "\tGot pad1 cluster with centroid "
                          << cluster1.getCentroid();
        }
        if ((fabs(cluster1.getCentroid() - centroid) < max_delta_ &&
            centroid < vert_bar_start_idx_) ||
            (centroid >= vert_bar_start_idx_ && cluster1.getCentroid() >= vert_bar_start_idx_ &&
            seed.getCentroidX() == cluster1.getCentroidX())) { 
            // use geometry y overlap scheme to see if this is really a match in x
            // should be done in a map

          if (centroid >= vert_bar_start_idx_ &&
              seed.getCentroidY() < cluster1.getCentroidY()) {
            // impossible combination
            if (verbose_ > 1) {
              ldmx_log(debug) << "\tSkipping impossible x cluster combination "
                                 "with y flags (tag up) ("
                              << seed.getCentroidY() << " "
                              << cluster1.getCentroidY() << ")";
            }
            continue;
          }

          // else: first (possible) match! loop through next pad too

          if (verbose_ > 1) {
            ldmx_log(debug) << "\t\tIt is close enough!. Check pad2";
          }

          // try making third pad clusters an optional part of track

          std::vector<ldmx::TrigScintCluster> cluster_vec = {seed, cluster1};

          bool has_match_dn = false;

          for (const auto &cluster2 : clusters_pad2) {
            if (verbose_ > 1) {
              ldmx_log(debug) << "\tGot pad2 cluster with centroid "
                              << cluster2.getCentroid();
            }

            if ((fabs(cluster2.getCentroid() - centroid) < max_delta_ &&
                centroid < vert_bar_start_idx_) ||
                (centroid >= vert_bar_start_idx_ && cluster2.getCentroid() >= vert_bar_start_idx_ &&
                fabs(seed.getCentroidX() - cluster2.getCentroidX()) <= max_delta_vert)) { 
                        // use geometry y overlap scheme to see if this is really a match
                    // in x

              if (centroid >= vert_bar_start_idx_ &&
                  (seed.getCentroidY() < cluster2.getCentroidY() ||
                   cluster1.getCentroidY() >
                       cluster2.getCentroidY())) {  // impossible
                if (verbose_ > 1) {
                  ldmx_log(debug)
                      << "\tSkipping impossible x cluster combination with y "
                         "flags (tag up dn) ("
                      << seed.getCentroidY() << " " << cluster1.getCentroidY()
                      << " " << cluster2.getCentroidY() << ")";
                }
                continue;
              }

              // first match! loop through next pad too

              if (verbose_ > 1) {
                ldmx_log(debug) << "\t\tIt is close enough!. Make a track";
              }

              // only make this vector now! this ensures against hanging
              // clusters with indices from earlier in the loop
              std::vector<ldmx::TrigScintCluster> three_cluster_vec = {
                  seed, cluster1, cluster2};

              /*
              // here we could break if we didn't want to allow all possible
              combinations madeTrack=true; break; //we're done with this
              iteration once there's a track made
              */
              // make a track
              ldmx::TrigScintTrack track = makeTrack(three_cluster_vec);
              track_candidates.push_back(track);
              has_match_dn = true;
            }  // if match in pad2
          }  // over clusters in pad2
          // if there was no match to this in pad 2, make a track with just
          // these two clusters
          if (!has_match_dn && skip_last_) {
            // we allow skipping last pad if needed
            ldmx::TrigScintTrack track = makeTrack(cluster_vec);
            track_candidates.push_back(track);
          }

        }  // if possible (x,)y match in pad1
                /*
        //same here
        if (madeTrack)
        break;
        */

      }  // over clusters in pad1

      // continue to next seed if 0 track candidates
      if (track_candidates.size() == 0) continue;

      int keep_idx = 0;
      float min_residual = 1000;  // some large number

      // no need to choose between only one candidate track
      if (track_candidates.size() > 1) {
        // now for each seed, pick only the track with the smallest residual.

        if (verbose_) {
          ldmx_log(debug) << "Got " << track_candidates.size()
                          << " tracks to check.";
        }

        for (uint idx = 0; idx < track_candidates.size(); idx++) {
          if ((track_candidates.at(idx)).getResidual() < min_residual) {
            keep_idx = (int)idx;
            min_residual =
                (track_candidates.at(idx)).getResidual();  // update minimum

            if (verbose_ > 1) {
              ldmx_log(debug)
                  << "Track at index " << idx
                  << " has smallest residual so far: " << min_residual;
            }

          }  // finding min residual
        }  // over track candidates
      }  // if more than 1 to choose from

      // store the track at keepIdx, if there was one we made it this far and
      // keepIdx is 0 or has been updated to the smallest residual track idx
      //	if (keepIdx >= 0) {
      tracks_.push_back(track_candidates.at(keep_idx));
      if (verbose_) {
        ldmx_log(debug) << "Kept track at index " << keep_idx;
        ldmx_log(trace) << track_candidates.at(keep_idx);
      }
      //}
    }  // over seeds

    // done here if there were no tracks found
    if (tracks_.size() == 0) {
      if (verbose_) {
        ldmx_log(debug) << "No tracks found!";
      }
      std::vector<ldmx::TrigScintTrack> empty{};
      event.add(output_collection_, empty);
      return;
    }
    // now, if there are multiple seeds sharing the same downstream hits, this
    // should also be remedied with a selection on min residual.

    // The logic of this loop kind of assumes I can remove tracks immediately --
    // that way I can do pairwise checks between more tracks within a single
    // loop. But for now I haven't figured out how to erase elements in a fool
    // proof way. So I iterate over a vector...

    std::vector keep_indices(tracks_.size(), 1);
    if (verbose_ > 1)
      ldmx_log(debug) << "vector of indices to keep has size "
                      << keep_indices.size();

    for (uint idx = tracks_.size() - 1; idx > 0; idx--) {
      // since we start in one end, we only have to check matches in one
      // direction
      ldmx::TrigScintTrack track = tracks_.at(idx);
      for (int idx_comp = idx - 1; idx_comp >= 0; idx_comp--) {
        if (verbose_ > 1)
          ldmx_log(debug) << "In track disambiguation loop, idx points at "
                          << idx << " and prev idx points at " << idx_comp;

        ldmx::TrigScintTrack next_track = tracks_.at(idx_comp);

        // no need to start pulling constituents from tracks that are
        // ridiculously far apart
        if (((fabs(track.getCentroid() - next_track.getCentroid()) <
            3 * max_delta_) && (track.getCentroid()<vert_bar_start_idx_)) //for the horizontal bars
            || ((fabs(track.getCentroidX() - next_track.getCentroidX()) < 2*max_delta_vert)
          && (track.getCentroidY() == next_track.getCentroidY()) && (track.getCentroid()>=vert_bar_start_idx_))) { 
            //and for the vertical bars, check if they are in the same quad and close enough
          std::vector<ldmx::TrigScintCluster> consts_1 =
              track.getConstituents();
          std::vector<ldmx::TrigScintCluster> consts_2 =
              next_track.getConstituents();
          if (verbose_ > 1)
            ldmx_log(debug)
                << "In track disambiguation loop, got the two tracks, "
                   "with nConstituents "
                << consts_1.size() << " and " << consts_2.size()
                << ", respectively. ";
          // let's do "if either cluster is shared" right now... but could also
          // have it settable to use a stricter cut: an AND
          if (((consts_1[1].getCentroid() == consts_2[1].getCentroid() ||
              ((consts_1.size() > 2) && (consts_2.size() > 2) &&
               (consts_1[2].getCentroid() == consts_2[2].getCentroid()))) && (track.getCentroid()<vert_bar_start_idx_)) || 
               //horizontal bars
              ((track.getCentroid()>=vert_bar_start_idx_) && ((consts_1[1].getCentroidX() == consts_2[1].getCentroidX()) ||
              (consts_1[2].getCentroidX() == consts_2[2].getCentroidX()) 
              || (consts_1[0].getCentroidX() == consts_2[0].getCentroidX())))) { //and vertical bars
                 // we have overlap downstream of the
            // seeding pad. probably, one cluster
            // in seeding pad is noise

            if (verbose_ > 1) {
              ldmx_log(debug) << "Found overlap! Tracks at index " << idx
                              << " and " << idx_comp;
              ldmx_log(trace) << tracks_.at(idx);
              ldmx_log(trace) << tracks_.at(idx_comp);
            } // 110 0.67 112 1.33

            if (((fabs((tracks_.at(idx)).getResidualX() -(tracks_.at(idx_comp)).getResidualX()))< 0.01) //it should be equal 
            && (track.getCentroid()>=vert_bar_start_idx_)){ //specific case for the vertical bars
              continue; //currently we can't do more here
            } else if (((tracks_.at(idx)).getResidual() <(tracks_.at(idx_comp)).getResidual() && (track.getCentroid()<vert_bar_start_idx_)) ||
            ((tracks_.at(idx)).getResidualX() <(tracks_.at(idx_comp)).getResidualX() && (track.getCentroid()>=vert_bar_start_idx_))) {
              // next track (lower index) is a worse choice, remove its flag for
              // keeping
              keep_indices.at(idx_comp) = 0;
            } else  // prefer next track over current. remove current track's
                    // keep
              // flag
              keep_indices.at(idx) = 0;
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
        }  // over tracks close enough to share constituents
      }  // over constructed tracks at other indices, to match
    }  // over constructed tracks

    for (uint idx = 0; idx < tracks_.size(); idx++) {
      if (verbose_ > 1) {
        ldmx_log(debug) << "keep flag for idx " << idx << " is "
                        << keep_indices.at(idx);
      }
      if (keep_indices.at(idx)) {  // this hasn't been flagged for removal

        cleaned_tracks.push_back(tracks_.at(idx));

        if (verbose_) {
          ldmx_log(debug) << "After cleaning, keeping track at index " << idx
                          << ": Centroid = " << (tracks_.at(idx)).getCentroid()
                          << "; CentroidX = "
                          << (tracks_.at(idx)).getCentroidX()
                          << "; CentroidY = "
                          << (tracks_.at(idx)).getCentroidY()
                          << "; track PE = " << (tracks_.at(idx)).getPE()
                          << tracks_.at(idx);
        }
      }  // if index flagged for keeping
    }  // over all (uniquely seeded) tracks in the event

    if (verbose_) {
      for (uint idx = 0; idx < tracks_.size(); idx++) {
        ldmx_log(debug) << "Keeping track at index " << idx << ":"
                        << tracks_.at(idx);
      }
    }

    if (verbose_) {
      ldmx_log(debug) << "Running track x,y matching ";
    }

    if (cleaned_tracks.size() > 0) {
      matchXYTracks(cleaned_tracks);
      std::vector<ldmx::TrigScintTrack> matched_tracks =
          cleaned_tracks;  // don't know why this copying needs to happen but it
                           // does
      //	std::vector<ldmx::TrigScintTrack>  matchXYTracks( cleanedTracks
      //); 		std::vector<ldmx::TrigScintTrack> matchedTracks =
      // matchXYTracks( cleanedTracks );
      for (auto trk : matched_tracks) {
        /*	for (uint idx = 0; idx < tracks_.size(); idx++) {
                      if (verbose_ > 1) {
                      ldmx_log(debug) << "keep flag for idx " << idx << " is "
                                      << keepIndices.at(idx);
                                      }
                                      if (keepIndices.at(idx)) {  // this hasn't
      been flagged for removal
      //check if channel nb is above that of horizontal bars
      if (tracks_.at(idx).getCentroid() >= vert_bar_start_idx_)
        */
        if (trk.getCentroid() >= vert_bar_start_idx_)
          cleaned_tracks_x.push_back(trk);  // acks_.at(idx));
        else
          cleaned_tracks_y.push_back(trk);  // acks_.at(idx));
        //		cleanedTracksY.push_back(trk);
        if (verbose_ > 1) {
          float centr = trk.getCentroid();  // tracks_.at(idx).getCentroid(); //
          std::string coll_str = centr >= vert_bar_start_idx_ ? "X" : "Y";
          coll_str = output_collection_ + coll_str;
          ldmx_log(debug) << "saving track with centroid " << centr
                          << " to output track collection " << coll_str;
        }
        // }
      }
    }

  }  // if there are clusters in all pads
  else if (verbose_) {
    ldmx_log(info)
        << "Not all pads had clusters; (maybe) skipping tracking attempt";
  }

  if (verbose_) {
    ldmx_log(debug) << "Done with tracking step. ";
  }

  event.add(output_collection_, cleaned_tracks);
  //  event.add(output_collection_, matchedTracks);

  event.add(output_collection_ + "Y", cleaned_tracks_y);
  event.add(output_collection_ + "X", cleaned_tracks_x);

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
  float centroid_x = 0;
  float centroid_y = 0;
  float beam_efrac = 0;
  float pe = 0;
  for (uint i = 0; i < clusters.size(); i++) {
    centroid += (clusters.at(i)).getCentroid();
    centroid_x += (clusters.at(i)).getCentroidX();
    centroid_y += (clusters.at(i)).getCentroidY();
    tr.addConstituent(clusters.at(i));
    beam_efrac += (clusters.at(i)).getBeamEfrac();
    pe += (clusters.at(i)).getPE();
  }
  centroid /= clusters.size();
  centroid_x /= clusters.size();
  if (centroid >= vert_bar_start_idx_) {
    if (verbose_) {
      ldmx_log(debug)
          << " --  In makeTrack made vertical bar track with centroid  "
          << centroid << " and y flag sum " << centroid_y;
      // try commenting this to check if that helps with an out-of-bounds
      // problem
      //                    << " from clusters with y centroids";
      // for (uint i = 0; i < clusters.size(); i++)
      // ldmx_log(debug) << "\tpad " << i << ": centroidY "
      //				  << (clusters.at(i)).getCentroidY();
    }
    // then the sum of centroid y is 0, 2, 4 or 6
    // we have 4 divisions, so, the center of it should be divNb/8
    // (or rather, that's where channel nBars/8 begins)
    // and then a factor 2 for the zig-zag pattern
    centroid_y = (centroid_y + 1) * 2 * n_bars_y_ / 8.;
    // TODO: here we could instead just use quadrant indices 0-3 by dividing by
    // 2 but that would mean that in the raw, x and y track centroidY would mean
    // different things
    if (verbose_) ldmx_log(debug) << " --  new centroidY = " << centroid_y;
  } else
    centroid_y /= clusters.size();

  beam_efrac /= clusters.size();
  pe /= clusters.size();

  float residual = 0;
  for (uint i = 0; i < clusters.size(); i++)
    residual += ((clusters.at(i)).getCentroid() - centroid) *
                ((clusters.at(i)).getCentroid() - centroid);
  residual = sqrt(residual / clusters.size());


  float residual_x = 0; //only for the vertical bars
  if (centroid>=vert_bar_start_idx_) {
    for (uint i = 0; i < clusters.size(); i++)
      residual_x += ((clusters.at(i)).getCentroidX() - centroid_x) *
                ((clusters.at(i)).getCentroidX() - centroid_x);
    residual_x = sqrt(residual_x / clusters.size());
  }


  tr.setResidualX(residual_x);
  tr.setCentroid(centroid);
  tr.setCentroidX(centroid_x);
  tr.setCentroidY(centroid_y);
  tr.setResidual(residual);
  tr.setBeamEfrac(beam_efrac);
  tr.setPE(pe);

  if (verbose_) {
    ldmx_log(debug) << " --  In makeTrack made track with centroid  "
                    << centroid << " and residual " << residual << " and pe "
                    << pe << " from clusters with centroids";
    for (uint i = 0; i < clusters.size(); i++)
      ldmx_log(debug) << "\tpad " << i << ": centroid "
                      << (clusters.at(i)).getCentroid();
  }

  return tr;
}

// std::vector<ldmx::TrigScintTrack>  TrigScintTrackProducer::matchXYTracks(
void TrigScintTrackProducer::matchXYTracks(
    std::vector<ldmx::TrigScintTrack> &tracks) {
  // map quadrant nb to track (can be multiple per quadrant)
  std::multimap<int, int>
      y_idx_quad_map;  // key = quad, val = track index in collection
  std::multimap<int, int> x_idx_quad_map;

  std::multimap<int, ldmx::TrigScintTrack> y_quad_map;
  std::multimap<int, ldmx::TrigScintTrack> x_quad_map;
  // map track in quadrant back to index in entire track collection
  // used for updating collection track variables
  std::map<ldmx::TrigScintTrack, int> y_track_map;
  std::map<ldmx::TrigScintTrack, int> x_track_map;

  uint trk_idx = -1;
  for (auto trk : tracks) {
    trk_idx++;
    // 1. get the y bar tracks with centroidX = -1
    if (trk.getCentroidX() == -1) {
      if (verbose_)
        ldmx_log(debug) << " --  In matchXYTracks found y track at "
                        << trk.getCentroidY() << "; mapping to quad "
                        << (int)trk.getCentroidY() / (n_bars_y_/2) << " with trk index "
                        << trk_idx;
      // 2. order them... or map them to quadrants. note that there are 2 layers
      // so 2*n_bars_y_/4 channels per quadrant
      y_quad_map.insert(std::make_pair((int)(trk.getCentroidY() / (n_bars_y_/2)), trk));
      y_track_map[trk] = trk_idx;
      y_idx_quad_map.insert(
          std::make_pair((int)(trk.getCentroidY() / (n_bars_y_/2)), trk_idx));

    } else {  // 3. get the remaining tracks (from vertical bars) and map them
              // (back) to (middle of) quadrants
      x_quad_map.insert(std::make_pair((int)(trk.getCentroidY() / (n_bars_y_/2)), trk));
      x_track_map[trk] = trk_idx;
      x_idx_quad_map.insert(
          std::make_pair((int)(trk.getCentroidY() / (n_bars_y_/2)), trk_idx));
      if (verbose_)
        ldmx_log(debug) << " --  In matchXYTracks found x track at (x,y) = ("
                        << trk.getCentroidX() << ", " << trk.getCentroidY()
                        << "); mapping to quad " << (int)trk.getCentroidY() / (n_bars_y_/2)
                        << " with trk index " << trk_idx;
    }
  }

  // 4a
  //
  // 1) here use the geometry? if we can assume perfect alignment we can take
  // width and nBars and take nBars/2 as origin
  // --- now do the matching ---

  // if there is no useful matching to be done: these are the pad width wide
  // numbers
  float x0 = 0;
  // this should be half the pad... could also set
  // it to full beam spot width
  float sx0 = fabs(x_start_); 
  float sx0_=fabs(bar_length_y_/2); // When there are no hits along the vertical bars

  // y_start_ is half the pad, so this should be half a quadrant
  float sy0 = fabs(y_start_) / 4.;

  // assume at least one y track. will have to figure out if there is ever a
  // reason to use an isolated x track in its place.
  for (auto yitr = y_quad_map.begin(); yitr != y_quad_map.end(); ++yitr) {
    int n_yin_quad = y_quad_map.count((*yitr).first);
    int n_xin_quad = x_quad_map.count((*yitr).first);
    float y{-9999.}, sy{-9999.}, x{-9999.}, x1{-9999.}, x2{-9999.}, sx1{-9999.},
        sx2{-9999.}, y1{-9999.}, y2{-9999.}, sy1{-9999.}, sy2{-9999.};
    // quad midpoint:
    float y0 = (((*yitr).first * 8)*y_conv_factor_ )+y_start_+ sy0; 
    float sx = 1. / 2 *
               x_conv_factor_;  // rely on x precision being one single bar
                                // width; always used unless x is undeterminable

    // check all x first
    // do the easiest first:
    if (n_xin_quad == 0) {  // then there's no hope of setting a better x here
      // just use the beam spot width... and center of pad
      x = x0;
      sx = sx0_;
      if (verbose_)
        ldmx_log(debug) << "\t\t\t no x info in quad " << (*yitr).first
                        << "; will set x to middle of pad, pad half-width as "
                           "precision: set (x, sx)=("
                        << x << ", " << sx << ")";
    }  // 0 x tracks in quadrant
    else if (n_xin_quad ==
             1) {  // slightly harder: 1 x track -- might be easy if
                   // it's just one y track; if several, need to
                   // think about overlaps. but in overlap case, just
                   // revert to setting x0 and sx0, when we know
      auto xitr = x_quad_map.find((*yitr).first);
      x = ((*xitr).second).getCentroidX() * x_conv_factor_ + x_start_;

      if (verbose_)
        ldmx_log(debug) << "\t\t\t 1 x in quad " << (*yitr).first
                        << ", getting (x, sx)=(" << x << ", " << sx << ")";
    }  // 1 x track in quadrant
    else if (n_xin_quad == 2) {  // finally if we have two tracks, get x1 and x2
                                 // and decide later how to use them
      // don't think we want to experiment with discerning three overlapping
      // tracks, so not >= 2
      //		  continue; //debugging: skip for now -- didn't help
      auto xitr1 = x_quad_map.lower_bound((*yitr).first);
      auto xitr2 = x_quad_map.upper_bound((*yitr).first);
      xitr2--;  // upper_bound points to next element

      if (xitr1 != xitr2) {  // should be true already but...
        x1 = ((*xitr1).second).getCentroidX() * x_conv_factor_ + x_start_;
        x2 = ((*xitr2).second).getCentroidX() * x_conv_factor_ + x_start_;
        sx1 = x_conv_factor_ / 2.;  // 1 bar width
        sx2 = sx1;
        x = (x1 + x2) / 2.;
        sx = fabs(x1 - x2) / 2;  // Ricardo: REMOVE the *x_conv_factor -- ,rely on x precision being one single pad width
        if (verbose_)
          ldmx_log(debug) << "\t\t -- 2 x in quad: setting y track x "
                             "coordinate to midpoint";
      }
    }  // if 2 x tracks in quad

    if (n_xin_quad >= 3) {  // no implementaion made so far
      x = x0;
      sx = sx0;
      if (verbose_)
        ldmx_log(debug) << "\t\t\t no x info in quad " << (*yitr).first
                        << "; will set x to middle of pad, pad half-width as "
                           "precision: set (x, sx)=("
                        << x << ", " << sx << ")";
    }  // 3 x tracks in quadrant

    // ok! over y:
    // can skip 0 y case by construction
    if (n_yin_quad == 1) {  // we can already now tell what the y coordinate and
                            // its precision is
      y = ((*yitr).second).getCentroidY() * y_conv_factor_ + y_start_;
      sy = ((*yitr).second).getResidual() * y_conv_factor_;
      // if all clusters lined up, assign
      // precision of 1 bar width
      if (sy == 0) sy = 1. / 2 * y_conv_factor_;

      if (n_xin_quad <= 1) {
        // 4. every quadrant which just has one of each --> done ;
        // b) set the sx, sy of the x track now, using the residuals from the
        // other b1) special case: no x tracks; then x, sx have been set above
        if (n_xin_quad==1){
          auto xidx = x_idx_quad_map.find((*yitr).first);
          tracks.at((*xidx).second).setPosition(x, y);
          tracks.at((*xidx).second).setSigmaXY(sx, sy);
        }
        if (verbose_)
          ldmx_log(debug) << "\t\t\t in quad " << (*yitr).first
                          << ", set (x, y) = (" << x << ", " << y
                          << ") and (sx, sy) = " << sx << ", " << sy << ")";
      auto yidx = y_idx_quad_map.find((*yitr).first);
      tracks.at((*yidx).second).setPosition(x, y);
      tracks.at((*yidx).second).setSigmaXY(sx, sy);
      continue;
      }
    }  // 1 y, 0 or 1 or 3+ x track in quadrant

    if (verbose_)
      ldmx_log(debug) << "\t\t in quad " << (*yitr).first
                      << ", not single x,y tracks: " << n_xin_quad
                      << " of x and " << n_yin_quad << " of y";

    if (n_yin_quad == 2) {  // let's start here and see if we can do >= 2 later
      // here one could do sth to avoid checking the other y track again in the
      // outermost loop over y      
      auto yitr1 = y_quad_map.lower_bound((*yitr).first);
      auto yitr2 = y_quad_map.upper_bound((*yitr).first);
      yitr2--;  // back up once
      y1 = ((*yitr1).second).getCentroidY() * y_conv_factor_ + y_start_;
      y2 = ((*yitr2).second).getCentroidY() * y_conv_factor_ + y_start_;
      sy1 = ((*yitr1).second).getResidual() * y_conv_factor_;
      sy2 = ((*yitr2).second).getResidual() * y_conv_factor_;
      if (sy1 == 0) sy1 = 1. / 2 * y_conv_factor_;
      if (sy2 == 0) sy2 = 1. / 2 * y_conv_factor_;
      y = (y1 + y2) / 2.;
      sy = fabs(y1 - y2) / 2 ; //Ricardo: remove y_conv_factor
      if (verbose_)
        ldmx_log(debug)
            << "\t\t -- 2 y in quad: setting x track y coordinate to midpoint";
    }  // 2y in quad
    
    if ((n_xin_quad == 0 || n_xin_quad >= 3) && (n_yin_quad == 2)) { //not using the X tracks for now for >=3
      if (n_xin_quad == 0){
        if (verbose_)
          ldmx_log(debug)
            << "\t\t -- No x tracks but 2 y tracks in quad: unsual behaviour";
      }
      auto yidx1 = y_idx_quad_map.lower_bound((*yitr).first);
      auto yidx2 = y_idx_quad_map.upper_bound((*yitr).first);
      yidx2--;
      tracks.at((*yidx1).second).setPosition(x, y1);
      tracks.at((*yidx1).second).setSigmaXY(sx, sy1);
      tracks.at((*yidx2).second).setPosition(x, y2);
      tracks.at((*yidx2).second).setSigmaXY(sx, sy2);
      continue;
    }
    
    if (n_yin_quad == 1 &&
        n_xin_quad == 2) {  // don't think we want to experiment with discerning
                            // three overlapping tracks, so not >= 2

      // first: set the y track coordinates to x  = the mid of x tracks, y = y
      // of y track
      auto yidx = y_idx_quad_map.find((*yitr).first);
      tracks.at((*yidx).second).setPosition(x, y);
      tracks.at((*yidx).second).setSigmaXY(sx, sy);

      int min_overlap_pe = 250;
      if (((*yitr).second).getPE() < min_overlap_pe) {
        // can't tell, really, that either of these belong to the y track. so.
        // let them keep their own x coordinate but set y to quadrant midpoint,
        // with uncertainty +/- half quadrant width (1/8 of pad height)
        y = y0;
        sy = sy0;
        if (verbose_)
          ldmx_log(debug) << "\t\t -- Can't tell which x track should be "
                             "matched to single y track. Setting both x track "
                             "coordinates to y quadrant value:";
      }  // if can't assume overlap
      else if (verbose_)
        ldmx_log(debug) << "\t\t -- Found large PE count ("
                        << ((*yitr).second).getPE() << " > " << min_overlap_pe
                        << "), suggesting overlap! Setting both x track "
                           "coordinates to y track value:";

      // consider making two x tracks out if this one, and, anyway have to set
      // their average as the y track x cocordinate
      // EXPERIMENTAL : apply only to x tracks, which can be disregarded for
      // electron counting
      if (verbose_)
        ldmx_log(debug) << "\t\t --  (x1, x2, y) = (" << x1 << ", " << x2
                        << ", " << y << ") and (sx1, sx2, sy) = " << sx1 << ", "
                        << sx2 << ", " << sy << ")";

      // now set x track coordinates according to overlap check result
      auto xidx1 = x_idx_quad_map.lower_bound((*yitr).first);
      auto xidx2 = x_idx_quad_map.upper_bound((*yitr).first);
      xidx2--;  // upper_bound points to (last+1) element
      tracks.at((*xidx1).second).setPosition(x1, y);
      tracks.at((*xidx1).second).setSigmaXY(sx1, sy);
      tracks.at((*xidx2).second).setPosition(x2, y);
      tracks.at((*xidx2).second).setSigmaXY(sx2, sy);

    }  // 1 y, 2 x tracks in the quadrant
    else if (n_yin_quad == 2 && n_xin_quad == 1) {
      // 5b) if there are more y than x: could be an overlap

      // first: set the x track coordinates to x = x of x track, y = the mid of
      // y tracks
      auto xidx = x_idx_quad_map.find((*yitr).first);
      tracks.at((*xidx).second).setPosition(x, y);
      tracks.at((*xidx).second).setSigmaXY(sx, sy);

      auto xitr = x_quad_map.lower_bound((*yitr).first);
      int min_overlap_pe = 300;
      if (((*xitr).second).getPE() < min_overlap_pe) {
        if (verbose_)
          ldmx_log(debug)
              << "\t\t just 1 x track with not-unusual PE in the quad -- can't "
                 "match; setting mid-point values for x ";
        x = x0;
        sx = sx0;
      }  // if can't assume overlap
      else {
        // consider making two x tracks out if this one, and, anyway have to set
        // their average as the y track x cocordinate
        // EXPERIMENTAL : apply only to x tracks, which can be disregarded for
        // electron counting
        if (verbose_)
          ldmx_log(debug) << "\t\t -- Found large PE count ("
                          << ((*xitr).second).getPE() << " > " << min_overlap_pe
                          << ") in x track, suggesting overlap! Setting both y "
                             "track coordinates to x track value:";
      }  // if can assume overlap
      if (verbose_)
        ldmx_log(debug) << "\t\t --  (x, y1, y2) = (" << x << ", " << y1 << ", "
                        << y2 << ") and (sx, sy1, sy2) = " << sx << ", " << sy1
                        << ", " << sy2 << ")";

      auto yidx1 = y_idx_quad_map.lower_bound((*yitr).first);
      auto yidx2 = y_idx_quad_map.upper_bound((*yitr).first);
      yidx2--;  // upper_bound points to next element
      tracks.at((*yidx1).second).setPosition(x, y1);
      tracks.at((*yidx1).second).setSigmaXY(sx, sy1);
      tracks.at((*yidx2).second).setPosition(x, y2);
      tracks.at((*yidx2).second).setSigmaXY(sx, sy2);

    }  // 2 y and 1 x track in quad
    else if (n_yin_quad == 2 && n_xin_quad == 2) {
      // MIDPONTS ALL OVER!
      auto xidx1 = x_idx_quad_map.lower_bound((*yitr).first);
      auto xidx2 = x_idx_quad_map.upper_bound((*yitr).first);
      xidx2--;
      auto yidx1 = y_idx_quad_map.lower_bound((*yitr).first);
      auto yidx2 = y_idx_quad_map.upper_bound((*yitr).first);
      yidx2--;

      if (y_idx_quad_map.find((*yitr).first) == y_idx_quad_map.end())
        ldmx_log(error) << "The two y tracks in the same quadrant at "
                        << (*yitr).first
                        << " appear to not be found in the y track map! "
                           "investigate. Note that yidx1.first = "
                        << (*yidx1).first
                        << " and  yidx2.first = " << (*yidx2).first;
      else {
        tracks.at((*xidx1).second).setPosition(x1, y);
        tracks.at((*xidx1).second).setSigmaXY(sx1, sy);
        tracks.at((*xidx2).second).setPosition(x2, y);
        tracks.at((*xidx2).second).setSigmaXY(sx2, sy);

        tracks.at((*yidx1).second).setPosition(x, y1);
        tracks.at((*yidx1).second).setSigmaXY(sx, sy1);
        tracks.at((*yidx2).second).setPosition(x, y2);
        tracks.at((*yidx2).second).setSigmaXY(sx, sy2);

        if (verbose_)
          ldmx_log(debug) << "\t\t -- in a 2 x 2 situaiton; midpoint y: " << y
                          << " for both x tracks, midpoint x: " << x
                          << " for both y tracks";
      }
    }  // if 2 y, 2 x tracks

    if (n_xin_quad > 2) {
      if (verbose_)
        ldmx_log(debug) << "\t\t -*-*-*- more than 2 x tracks in the same quad "
                           "-- nothing done about the x,y coordinates in this "
                           "situation -- implement if needed!!";
    }
    if (n_yin_quad > 2) {
      if (verbose_)
        ldmx_log(debug) << "\t\t -*-*-*- more than 2 y tracks in the same quad "
                           "-- nothing done about the x,y coordinates in this "
                           "situation -- implement if needed!!";
    }

  }  // over y tracks

  y_quad_map.clear();
  x_quad_map.clear();

  //  return tracks;
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
DECLARE_PRODUCER(trigscint::TrigScintTrackProducer);
