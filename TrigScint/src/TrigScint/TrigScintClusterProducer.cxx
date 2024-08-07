
#include "TrigScint/TrigScintClusterProducer.h"

#include <iterator>  // std::next
#include <map>

namespace trigscint {

void TrigScintClusterProducer::configure(framework::config::Parameters &ps) {
  seed_ = ps.getParameter<double>("seed_threshold");
  minThr_ = ps.getParameter<double>("clustering_threshold");
  maxWidth_ = ps.getParameter<int>("max_cluster_width");
  input_collection_ = ps.getParameter<std::string>("input_collection");
  passName_ = ps.getParameter<std::string>("input_pass_name");
  output_collection_ = ps.getParameter<std::string>("output_collection");
  verbose_ = ps.getParameter<int>("verbosity");
  vertBarStartIdx_ = ps.getParameter<int>("vertical_bar_start_index");
  timeTolerance_ = ps.getParameter<double>("time_tolerance");
  padTime_ = ps.getParameter<double>("pad_time");
  // electronPESeparation_ = ps.getParameter<int>("electron_pe_separation");
  electronEnergySeparation_ = ps.getParameter<double>("electron_energy_spearation");

  // Bar sizes
  barWidth_x_ = ps.getParameter<double>("vertical_bar_width");
  barWidth_y_ = ps.getParameter<double>("horizontal_bar_width");
  barDepth_z_ = ps.getParameter<double>("bar_depth");
  // Bar gaps
  barGap_x_ = ps.getParameter<double>("vertical_bar_gap");
  barGap_y_ = ps.getParameter<double>("horizontal_bar_gap");
  barGap_z_ = ps.getParameter<double>("depth_bar_gap");
  // Number of horizontal and vertical bars
  nBarsX_ = ps.getParameter<int>("number_vertical_bars");
  nBarsY_ = ps.getParameter<int>("number_horizontal_bars");

  if (verbose_) {
    ldmx_log(info) << "In TrigScintClusterProducer: configure done!";
    ldmx_log(info) << "Got parameters: \nSeed threshold:   " << seed_
                   << "\nClustering threshold: " << minThr_
                   << "\nMax cluster width: " << maxWidth_
                   << "\nExpected pad hit time: " << padTime_
                   << "\nMax hit time delay: " << timeTolerance_
                   << "\nVertical bar start index:     " << vertBarStartIdx_
                   << "\nInput collection:     " << input_collection_
                   << "\nInput pass name:     " << passName_
                   << "\nOutput collection:    " << output_collection_
                   << "\nVerbosity: " << verbose_;
  }

  return;
}

void TrigScintClusterProducer::produce(framework::Event &event) {
  // parameters.
  // a cluster seeding threshold
  // a clustering threshold -- a lower boundary for being added at all (zero
  // suppression)
  // a maximum cluster width
  //
  // steps.
  // 1. get an input collection of digi hits. at most one entry per channel.
  // 2. access them by channel number
  // 3. clustering:
  //       a. add first hit > seedThr to cluster(ling) . store content as
  //       localMax. track size of cluster (1) b. if not in beginning of array:
  //       add cell before while content <  add next hit first hit > seedThr to
  //       cluster(ling) b. while content <  add next hit first hit > seedThr to
  //       cluster(ling)

  /*

    The trigger pad geometry considered for this clustering algorithm is:


    |   |   |   |   |   |
    | 0 | 2 | 4 | 6 | 8 |  ... | 48|

      | 1 | 3 | 5 | 7 | 9 |  ... | 49|
      |   |   |   |   |   |

    with hits in channels after digi looking something like this


    ampl:      _                   _                   _
              | |_                | |                 | |
          ----| | |---------------| |-----------------| |------- cluster seed
    threshold | | |_              | |_ _              | |_
             _| | | |            _| | | |            _| | |  _
            | | | | |     vs    | | | | |    vs     | | | | | |


                |                   |                   |
          split | seeds        keep | disregard   keep! | just move on
                | next cl.          | (later seed       | (no explicit splitting)
                                    might pick
                                    it up)


    The idea being that while there could be good reasons for an electron to
    touch three pads in a row, there is no good reason for it to cross four.
    This is noise, or, the start of an adjacent cluster. In any case, 4 is not a
    healthy cluster. Proximity to a seed governs which below-seed channels to
    include. By always starting in one end but going back (at most two
    channels), this algo guarantees symmetric treatment on both sides of the
    seed.


    //Procedure: keep going until there is a seed. walk back at most 2 steps
    // add all the hits. clusters of up to 3 is fine.
    // if the cluster is > 3, then we need to do something.
    // if it's == 4, we'd want to split in the middle if there are two potential
    seeds. retain only the first half, cases are (seed = s, n - no/noise)
    // nsns , nssn, snsn, ssnn.  nnss won't happen (max 1 step back from s,
    unless there is nothing in front)
    // all these are ok to split like this bcs even if ssnn--> ss and some small
    nPE is lost, that's probably pretty negligible wrt the centroid position,
    with two seeds in one cluster

    // if it's > 4, cases are
    // nsnsn, nsnss, nssnn, nssns, snsnn, snsns, ssnnn, ssnns.
    // these are also all ok to just truncate after 2. and then the same check
    outlined above will happen to the next chunk.

    // so in short we can
    // 1. seed --> addHit
    // 2. walk back once --> addHit
    // 3. check next: if seed+1 exists && seed +2 exists,
    // 3a. if seed-1 is in already, stop here.
    // 3b. else if seed+3 exists, stop here.
    // 3c. else addHit(seed+1), addHit(seed+2)
    // 4. if seed+1 and !seed+2 --> addHit(seed+1)
    // 5. at this point, if clusterSize is 2 hits and seed+1 didn't exist, we
    can afford to walk back one more step and add whatever junk was there (we
    know it's not a seed)


    */

  if (verbose_) {
    ldmx_log(debug)
        << "TrigScintClusterProducer: produce() starts! Event number: "
        << event.getEventHeader().getEventNumber();
  }

  // looper over digi hits and aggregate energy depositions for each detID

  const auto digis{
      event.getCollection<ldmx::TrigScintHit>(input_collection_, passName_)};

  if (verbose_) {
    ldmx_log(debug) << "Got digi collection " << input_collection_ << "_"
                    << passName_ << " with " << digis.size() << " entries ";
  }

  // TODO remove this once verified that the noise overlap bug is gone
  bool doDuplicate = true;

  // 1. store all the channel digi content in channel order
  auto iDigi{0};
  for (const auto &digi : digis) {
    // these are unordered hits, and this collection is zero-suppressed
    // map the index of the digi to the channel index

    if (digi.getPE() >
        minThr_) {  // cut on a min threshold (for a non-seeding hit to be added
                    // to seeded clusters) already here

      int ID = digi.getBarID();

      // first check if there is a (pure) noise hit at this channel,  and
      // replace it in that case. this is a protection against a problem that
      // shouldn't be there in the first place.
      if (doDuplicate && hitChannelMap_.find((ID)) != hitChannelMap_.end()) {
        int idx = ID;
        std::map<int, int>::iterator itr = hitChannelMap_.find(idx);
        double oldVal = digis.at(itr->second).getPE();
        if (verbose_) {
          ldmx_log(debug) << "Got duplicate digis for channel " << idx
                          << ", with already inserted value " << oldVal
                          << " and new " << digi.getPE();
        }
        if (digi.getPE() > oldVal) {
          hitChannelMap_.erase(itr->first);
          if (verbose_) {
            ldmx_log(debug)
                << "Skipped duplicate digi with smaller value for channel "
                << idx;
          }
        }
      }

      // don't add in late hits
      if (digi.getTime() > padTime_ + timeTolerance_) continue;

      hitChannelMap_.insert(std::pair<int, int>(ID, iDigi));
      // the channel number is the key, the digi list index is the value

      if (verbose_) {
        ldmx_log(debug) << "Mapping digi hit nb " << iDigi
                        << " with energy = " << digi.getEnergy()
                        << " MeV, nPE = " << digi.getPE() << " > " << minThr_
                        << " to key/channel " << ID;
      }
    }
    iDigi++;
  }

  // 2. now step through all the channels in the map and cluster the hits

  std::map<int, int>::iterator itr;

  // Create the container to hold the digitized trigger scintillator hits.
  std::vector<ldmx::TrigScintCluster> trigScintClusters;

  // loop over channels
  for (itr = hitChannelMap_.begin(); itr != hitChannelMap_.end(); ++itr) {
    // this hit may have disappeared
    if (hitChannelMap_.find(itr->first) == hitChannelMap_.end()) {
      if (verbose_ > 1) {
        ldmx_log(debug) << "Attempting to use removed hit at channel "
                        << itr->first << "; skipping.";
      }
      continue;
    }

    // i don't like this but for now, erasing elements in the map leads, as it
    // turns out, to edge cases where i miss out on hits or run into
    // non-existing indices. so while what i do below means that i don't need to
    // erase hits, i'd rather find a way to do that and skip this book keeping:
    bool hasUsed = false;
    for (const auto &index : v_usedIndices_) {
      if (index == itr->first) {
        if (verbose_ > 1) {
          ldmx_log(warn) << "Attempting to re-use hit at channel " << itr->first
                         << "; skipping.";
        }
        hasUsed = true;
      }
    }
    if (hasUsed) continue;
    if (verbose_ > 1) {
      ldmx_log(debug) << "\t At hit with channel nb " << itr->first << ".";
    }

    if (hitChannelMap_.size() == 0)  // we removed them all..? shouldn't ever happen
    {
      if (verbose_)
        ldmx_log(warn) << "Time flies, and all clusters have already been "
                          "removed! Unclear how we even got here; interfering "
                          "here to get out of the loop. ";
      break;
    }

    ldmx::TrigScintHit digi = (ldmx::TrigScintHit)digis.at(itr->second);

    // skip all until hit a seed
    if (digi.getPE() >= seed_) {
      if (verbose_ > 1) {
        ldmx_log(debug) << "Seeding cluster with channel " << itr->first
                        << "; content " << digi.getPE();
      }

      // 1.  add seeding hit to cluster

      addHit(itr->first, digi);

      if (verbose_ > 1) {
        ldmx_log(debug) << "\t itr is pointing at hit with channel nb "
                        << itr->first << ".";
      }

      // ----- first look back one step

      // we have added the hit from the neighbouring channel to the list only if
      // it's above clustering threshold so no check needed now
      std::map<int, int>::iterator itrBack =
          hitChannelMap_.find(itr->first - 1);

      bool hasBacked = false;

      if (itrBack !=
          hitChannelMap_.end()) {  // there is an entry for the previous
                                   // channel, so it had content above threshold
        // but it wasn't enough to seed a cluster. so, unambiguous that it
        // should be added here because it's its only chance to get in.

        // need to check again for backwards hits
        hasUsed = false;
        for (const auto &index : v_usedIndices_) {
          if (index == itrBack->first) {
            if (verbose_ > 1) {
              ldmx_log(warn) << "Attempting to re-use hit at channel "
                             << itrBack->first << "; skipping.";
            }
            hasUsed = true;
          }
        }
        if (!hasUsed) {
          digi = (ldmx::TrigScintHit)digis.at(itrBack->second);

          // 2. add seed-1 to cluster
          addHit(itrBack->first, digi);
          hasBacked = true;

          if (verbose_ > 1) {
            ldmx_log(debug) << "Added -1 channel " << itrBack->first
                            << " to cluster; content " << digi.getPE();
            ldmx_log(debug) << "\t itr is pointing at hit with channel nb "
                            << itr->first << ".";
          }

        }  // if seed-1 wasn't used already
      }    // there exists a lower, unused neighbour

      // 3. check next: if seed+1 exists && seed +2 exists,
      //    3a. if seed-1 is in already, this is a case for a split, at seed. go
      //    directly to check on seed-2, don't add more here. 3b. else. addHit
      //    (seed+1) 3c. if seed+3 exists, this is a split, at seed+1. don't add
      //    more here. 3d. else addHit(seed+2)
      // 4. if seed+1 and !seed+2 --> go to addHit(seed+1)

      // --- now, step 3, 4: look ahead 1 step from seed

      if (v_addedIndices_.size() < maxWidth_) {
        // (in principle these don't need to be different iterators, but it
        // makes the logic easier to follow)
        std::map<int, int>::iterator itrNeighb =
            hitChannelMap_.find(itr->first + 1);
        if (itrNeighb !=
            hitChannelMap_.end()) {  // there is an entry for the next channel,
                                     // so it had content above threshold
          // seed+1 exists
          // check if there is sth in position seed+2
          if (hitChannelMap_.find(itrNeighb->first + 1) !=
              hitChannelMap_.end()) {  // a hit with that key exists, so seed+1
                                       // and seed+2 exist
            if (!hasBacked) {  // there is no seed-1 in the cluster. room for at
                               // least seed+1, and for seed+2 only if there is
                               // no seed+3
              // 3b
              digi = (ldmx::TrigScintHit)digis.at(itrNeighb->second);
              addHit(itrNeighb->first, digi);

              if (verbose_ > 1) {
                ldmx_log(debug)
                    << "No -1 hit. Added +1 channel " << itrNeighb->first
                    << " to cluster; content " << digi.getPE();
                ldmx_log(debug) << "\t itr is pointing at hit with channel nb "
                                << itr->first << ".";
              }

              if (v_addedIndices_.size() < maxWidth_) {
                if (hitChannelMap_.find(itrNeighb->first + 2) ==
                    hitChannelMap_
                        .end()) {  // no seed+3. also no seed-1. so add seed+2
                  // 3d.  add seed+2 to the cluster
                  itrNeighb = hitChannelMap_.find(itr->first + 2);
                  digi = (ldmx::TrigScintHit)digis.at(itrNeighb->second);
                  addHit(itrNeighb->first, digi);
                  if (verbose_ > 1) {
                    ldmx_log(debug)
                        << "No +3 hit. Added +2 channel " << itrNeighb->first
                        << " to cluster; content " << digi.getPE();
                    ldmx_log(debug)
                        << "\t itr is pointing at hit with channel nb "
                        << itr->first << ".";
                  }
                }

              }   // if no seed+3 --> added seed+2
            }     // if seed-1 wasn't added
          }       // if seed+2 exists. then already added seed+1.
          else {  // so: if not, then we need to add seed+1 here. (step 4)
            digi = (ldmx::TrigScintHit)digis.at(
                itrNeighb->second);  // itrNeighb hasn't moved since there was
                                     // no seed+2
            addHit(itrNeighb->first, digi);

            if (verbose_ > 1) {
              ldmx_log(debug)
                  << "Added +1 channel " << itrNeighb->first
                  << " as last channel to cluster; content " << digi.getPE();
              ldmx_log(debug) << "\t itr is pointing at hit with channel nb "
                              << itr->first << ".";
            }
          }
        }  // if seed+1 exists
        // 5. at this point, if clusterSize is 2 hits and seed+1 didn't exist,
        // we can afford to walk back one more step and add whatever junk was
        // there (we know it's not a seed)
        else if (hasBacked &&
                 hitChannelMap_.find(itrBack->first - 1) !=
                     hitChannelMap_
                         .end()) {  // seed-1 has been added, but not seed+1,
                                    // and there is a hit in seed-2
          itrBack = hitChannelMap_.find(itr->first - 2);
          digi = (ldmx::TrigScintHit)digis.at(itrBack->second);
          addHit(itrBack->first, digi);

          if (verbose_ > 1) {
            ldmx_log(debug) << "Added -2 channel " << itrBack->first
                            << " to cluster; content " << digi.getPE();
          }
          if (verbose_ > 1) {
            ldmx_log(debug) << "\t itr is pointing at hit with channel nb "
                            << itr->first << ".";
          }

        }  // check if add in seed -2

      }  // if adding another hit, going forward, was allowed

      // done adding hits to cluster. calculate centroid
      centroid_ /= val_;  // final weighting step: divide by total
      centroid_ -= 1;     // shift back to actual channel center

      ldmx::TrigScintCluster cluster;

      if (verbose_ > 1) {
        ldmx_log(debug) << "Now have " << v_addedIndices_.size()
                        << " hits in the cluster ";
      }
      cluster.setSeed(v_addedIndices_.at(0));
      cluster.setIDs(v_addedIndices_);
      cluster.setNHits(v_addedIndices_.size());
      cluster.setCentroid(centroid_);
      float cx;
      float cy = centroid_;
      float cz = -99999;  // set to nonsense for now. could be set to module nb
      if (centroid_ <
          vertBarStartIdx_)  // then in horizontal bars --> we don't know X
        cx = -1;  // set to nonsense in barID space. could translate to x=0 mm
      else {
        cx = (int)((centroid_ - vertBarStartIdx_) / 4);  // start at 0
        cy = (int)centroid_ % 4;
      }

      setPosition(cluster);     // calculates physical position of cluster in x,y,z [mm]

      cluster.setCentroidXYZ(cx, cy, cz);
      cluster.setEnergy(valE_);
      cluster.setPE(val_);
      cluster.setTime(time_ / val_);
      cluster.setBeamEfrac(beamE_ / valE_);
      // if (val_ / v_addedIndices_.size() >= electronPESeparation_) {
      //   cluster.setMaxElectrons(2);
      // }
      if (valE_ / v_addedIndices_.size() > electronEnergySeparation_) {
        cluster.setMaxElectrons(2);
      }

      centroid_ = 0;
      val_ = 0;
      valE_ = 0;
      beamE_ = 0;
      time_ = 0;
      v_addedIndices_.resize(
          0);  // book keep which channels have already been added to a cluster

      trigScintClusters.push_back(cluster);

      if (verbose_) cluster.Print();

      if (verbose_ > 1) {
        ldmx_log(debug)
            << "\t Finished processing of seeding hit with channel nb "
            << itr->first << ".";
      }

    }  // if content enough to seed a cluster

    if (hitChannelMap_.begin() == hitChannelMap_.end()) {
      if (verbose_)
        ldmx_log(warn) << "Time flies, and all clusters have already been "
                          "removed! Interfering here to get out of the loop. ";
      break;
    }
  }  // over channels

  if (trigScintClusters.size() > 0)
    event.add(output_collection_, trigScintClusters);

  hitChannelMap_.clear();
  v_usedIndices_.resize(
      0);  // book keep which channels have already been added to a cluster

  return;
}

void TrigScintClusterProducer::addHit(uint idx, ldmx::TrigScintHit hit) {
  float ampl = hit.getPE();
  val_ += ampl;
  float energy = hit.getEnergy();
  valE_ += energy;

  centroid_ += (idx + 1) * ampl;  // need non-zero weight of channel 0. shifting
                                  // centroid back by 1 in the end
  // this number gets divided by val at the end
  v_addedIndices_.push_back(idx);

  beamE_ += hit.getBeamEfrac() * energy;
  if (hit.getTime() > -990.) {
    time_ += hit.getTime() * ampl;
  }

  v_usedIndices_.push_back(idx);
  /*    // not working properly, but i'd prefer this type of solution
        hitChannelMap_.erase( idx ) ;
        if (verbose_ > 1 ) {
        ldmx_log(debug) << "Removed used hit " << idx << " from list";
        }
        if ( hitChannelMap_.find( idx) != hitChannelMap_.end() )
        std::cerr << "----- WARNING! Hit still present in map after removal!! ";
  */
  if (verbose_ > 1) {
    ldmx_log(debug) << "   In addHit, adding hit at " << idx
                    << " with amplitude " << ampl
                    << ", updating cluster to current centroid "
                    << centroid_ / val_ - 1 << " and energy " << val_
                    << ". index vector now ends with "
                    << v_addedIndices_.back();
  }
  
  return;
}

/* The method below takes geometry of the bars in the new trigScintSetup
  (two horizontal rows, one vertical row)
  and calculates the physical position of the cluster
  and stores it in member variables x_, y_ and z_ of the cluster
*/
void TrigScintClusterProducer::setPosition(ldmx::TrigScintCluster &cluster) {
      // Conversion factors x,y,z:
      // Horizontal:  *each bar only goes half this distance up (overlap/zig-zag)
      //                and starts at half height of bar
      // Vertical:    *each bar goes entire distance sideways (no overlap)
      //                and starts at half width of bar
      // Depth:       *each bar row separated by one bar depth and gap
      //                and starts surface of vertical bar
      double yConvFactor_ = (barWidth_y_ + barGap_y_) / 2.;
      double yStart_ = -(nBarsY_ * (barWidth_y_ + barGap_y_) - barGap_y_) / 2.;
      double xConvFactor_ = barWidth_x_ + barGap_x_;
      double xStart_ = -(nBarsX_ * (barWidth_x_ + barGap_x_) - barGap_x_) / 2.;		
      double zConvFactor_ = barDepth_z_ + barGap_z_;
      double zStart = (barDepth_z_ + barGap_z_ ) / 2.;

      double x = -99999.;             // Initialise x at nonsense value
      double y = -99999.;             // Initialise y at nonsense value
      double z = -99999.;							// Initialise z at nonsense value
      double sx = -99999.;            // Set uncertainty in x position
      double sy = -99999.;            // Set uncertainty in y position
      double sz = zConvFactor_ / 2.;  // Set uncertainty in z position

      centroid_ = cluster.getCentroid();  // Get cluster centroid

      if (centroid_ < vertBarStartIdx_) {		  // If we are looking at horizontal bar centroid
        sy = yConvFactor_ / 2.;  
        y = yStart_ + centroid_*yConvFactor_ +
            0.5*barWidth_y_;                    // calculate y
        // How many horizontal bars are hit
        int hitY = 0;							                // Initialize number of horizontal bars hit
        for (auto hitID : cluster.getHitIDs() ) { // Loop through all barID hits in the cluster
          if (hitID < vertBarStartIdx_) {			    // If horizontal bar
            hitY++;								                // Increase with 1
          }}
        
        // Set z-position
        if (hitY == 1) {							                        // if hit on 1 horizontal bar
          if (std::fmod(centroid_,2) == 0) {                    // if barID is even (front horizontal row)
            z = zStart - 2*zConvFactor_;                          // z-position is middle of front horizontal row
          } else {								                              // if barID is odd (back horizontal row)
            z = zStart - zConvFactor_;}				                    // z-position is middle of back horizontal row
        } else if (hitY == 2) {								                  // if hits 2 horizontal bars
          z = zStart - zConvFactor_*3/2;                          // z-position between horizontal rows
        } else {								                                // if hits 3 horizontal bars
          // The idea here is that if hitIDsum is even it must be 2 front row hits + 1 back row hits
          int hitIDsum = 0;						                          // Sum of IDs
          for (auto i : cluster.getHitIDs()) {hitIDsum += i;}     // Calculate sum
          
          if (hitIDsum%2 != 0) {						                      // If 1 front row hit + 2 back row hits
            z = zStart - zConvFactor_*4/3;                          // z-position is back row minus 1/3*(bar_width + bar_gap)
          } else {							                                  // If 2 front row hits + 1 back row hit
            z = zStart - zConvFactor_*5/3;                          // z-position is back row minus 2/3*(bar_width + bar_gap)
          }}
      } else {								                                  // if we are looking at vertical bar centroid
        sx = xConvFactor_ / 2.;
        int cx = std::floor((centroid_ - vertBarStartIdx_) / 4.); // conversion from centroid_ ID to barID (starting at 0)
        x = xStart_ + cx*xConvFactor_ + 0.5*barWidth_x_;          // calculate x
        z = zStart;                                               // calculate z
      }

      cluster.setPositionXYZ(x,y,z);        // Set cluster centroid position
      cluster.setSigmaXYZ(sx,sy,sz);        // Set uncertainty of cluster position

      return;
}

void TrigScintClusterProducer::onProcessStart() {
  ldmx_log(debug) << "Process starts!";

  return;
}

void TrigScintClusterProducer::onProcessEnd() {
  ldmx_log(debug) << "Process ends!";

  return;
}

}  // namespace trigscint

DECLARE_PRODUCER_NS(trigscint, TrigScintClusterProducer);
