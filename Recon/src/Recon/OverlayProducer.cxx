#include "Recon/OverlayProducer.h"

namespace recon {

void OverlayProducer::configure(framework::config::Parameters &parameters) {
  params_ = parameters;

  // name of file containing events to be overlaid, and a list of collections to
  // overlay
  overlay_filename_ = parameters.get<std::string>("overlay_filename");
  calo_collections_ =
      parameters.get<std::vector<std::string>>("calo_collections");
  tracker_collections_ =
      parameters.get<std::vector<std::string>>("tracker_collections");
  sim_passname_ = parameters.get<std::string>("sim_passname");
  overlay_passname_ = parameters.get<std::string>("overlay_passname");
  out_coll_postfix_ = parameters.get<std::string>("out_coll_postfix");
  // overlay specifics:
  poisson_mu_ = parameters.get<double>("poisson_mu");
  do_poisson_in_time_ = parameters.get<bool>("do_poisson_in_time");
  do_poisson_out_of_time_ = parameters.get<bool>("do_poisson_out_of_time");
  time_sigma_ = parameters.get<double>("time_sigma");
  time_mean_ = parameters.get<double>("time_mean");
  n_earlier_ = parameters.get<int>("n_earlier");
  n_later_ = parameters.get<int>("n_later");
  bunch_spacing_ = parameters.get<double>("bunch_spacing");
  start_event_min_ = parameters.get<int>("start_event_min");
  start_event_max_ = parameters.get<int>("start_event_max");

  /// Print the parameters actually set. Helpful in case of typos.
  ldmx_log(debug) << "Got parameters \n \t overlayFileName = "
                  << overlay_filename_
                  << "\n\t sim pass name = " << sim_passname_
                  << "\n\t overlay pass name = " << overlay_passname_;
  ldmx_log(debug) << "\n\t overlayCaloHitCollections = ";
  for (const auto &coll : calo_collections_) {
    ldmx_log(debug) << coll << "; ";
  }

  ldmx_log(debug) << "\n\t overlayTrackerHitCollections = ";
  for (const std::string &coll : tracker_collections_) {
    ldmx_log(debug) << coll << "; ";
  }

  ldmx_log(trace) << "\n\t numberOverlaidInteractions = " << poisson_mu_
                  << "\n\t nEarlierBunchesToSample = " << n_earlier_
                  << "\n\t nLaterBunchesToSample = " << n_later_
                  << "\n\t bunchSpacing = " << bunch_spacing_
                  << "\n\t doPoissonIntime = " << do_poisson_in_time_
                  << "\n\t doPoissonOutoftime = " << do_poisson_out_of_time_
                  << "\n\t timeSpread = " << time_sigma_
                  << "\n\t timeMean = " << time_mean_
                  << "\n\t startEventMin = " << start_event_min_
                  << "\n\t startEventMax = " << start_event_max_;

  return;
}  // end configure()

void OverlayProducer::onNewRun(const ldmx::RunHeader &) {
  /// set up random seeds
  const auto &rnss = getCondition<framework::RandomNumberSeedService>(
      framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
  rndm_ = std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndm"));
  rndm_time_ =
      std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndmTime"));

  // Pick a random event from the Pileup file
  int start_event = rndm_->Uniform(start_event_min_, start_event_max_);
  // EventFile::skipToEvent handles actual number of events in file
  int ev_number = overlay_file_->skipToEvent(start_event);
  if (ev_number < 0) {
    EXCEPTION_RAISE("BadRead", "Couldn't read to starting offset.");
  }
  overlay_event_.getEventHeader().setEventNumber(ev_number);
  ldmx_log(info) << "Starting overlay process with pileup event number "
                 << ev_number << " (random event number picked was "
                 << start_event << ").";
}  // end onNewRun()

void OverlayProducer::produce(framework::Event &event) {
  // using nextEvent to loop, we need to loop over overlay events and in an
  // inner loop, loop over collections, and store them. after all pileup events
  // have been added, the vector of collections is iterated over and added to
  // the event bus.
  std::map<std::string, std::vector<ldmx::SimCalorimeterHit>>
      calo_collection_map;
  std::map<std::string, std::vector<ldmx::SimTrackerHit>>
      tracker_collection_map;
  std::map<int, ldmx::SimCalorimeterHit> hit_map;

  // start by copying over all the collections from the sim event

  /* ----------- first do the SimCalorimeterHits ----------- */

  // get the calo hits collections that we want to overlay, by looping over
  // the list of collections passed to the producer : calo_collections_
  for (const auto &coll_name : calo_collections_) {
    // for now, Ecal and only Ecal uses contribs instead of multiple
    // simhits_calo per channel, meaning, it requires special treatment
    auto needs_contribs_added{
        coll_name.find("Ecal") != std::string::npos ? true : false};

    // start out by just copying the sim hits, unaltered.
    auto simhits_calo =
        event.getCollection<ldmx::SimCalorimeterHit>(coll_name, sim_passname_);
    // but don't copy ecal hits immediately: for them, wait until overlay
    // contribs have been added. then add everything through the hit_map
    if (!needs_contribs_added) {
      calo_collection_map[coll_name + out_coll_postfix_] = simhits_calo;
    }

    ldmx_log(debug) << "in loop: start of collection " << coll_name
                    << "in loop: printing current sim event: ";
    ldmx_log(debug) << "in loop: size of sim hits vector " << coll_name
                    << " is " << simhits_calo.size();

    // we don't need to touch the hard process sim hits, really... but we
    // might need the simhits in the hit map.
    if (needs_contribs_added) {
      for (const ldmx::SimCalorimeterHit &simhit : simhits_calo) {
        ldmx_log(trace) << simhit;
        // this copies the hit, its ID and its coordinates directly
        hit_map[simhit.getID()] = simhit;

      }  // over calo simhit collection
    }  // if needContribs

  }  // over calo collections for sim event

  /* ----------- now do the same with SimTrackerHits! ----------- */

  // get the SimTrackerHit collections that we want to overlay, by looping
  // over the list of collections passed to the producer : tracker_collections_
  for (const auto &coll_name : tracker_collections_) {
    auto simhits_tracker =
        event.getCollection<ldmx::SimTrackerHit>(coll_name, sim_passname_);
    tracker_collection_map[coll_name + out_coll_postfix_] = simhits_tracker;

    // the rest is printouts for debugging
    ldmx_log(debug) << "in loop: size of sim hits vector " << coll_name
                    << " is " << simhits_tracker.size();

    ldmx_log(debug) << "in loop: start of collection " << coll_name
                    << "in loop: printing current sim event: ";

    for (const ldmx::SimTrackerHit &simhit : simhits_tracker) {
      ldmx_log(trace) << simhit;
    }
  }  // over tracker collections for sim event

  /* ----------- now do the pileup overlay ----------- */

  // we could shift these by a random number, effectively placing the
  // sim event at random positions in the interval, preserving the
  // overall interval length
  // int simBunch=  static_cast<int>(rndm_time_->Uniform(
  //				   -(n_earlier_+1) , n_later_+1));  // +1 to get
  // inclusive interval
  int start_bunch = -n_earlier_;
  int end_bunch = n_later_;

  // TODO -- figure out if we should also randomly shift the time of the sim
  // event (likely only needed if time bias gets picked up by BDT or ML by way
  // of pulse behaviour)
  for (int bunch_offset{start_bunch}; bunch_offset <= end_bunch;
       bunch_offset++) {
    // sample a poisson distribution, or use mu as fixed number of overlay
    // events
    int n_events_overlay = do_poisson_out_of_time_
                               ? rndm_->Poisson(poisson_mu_)
                               : static_cast<int>(poisson_mu_);

    // special case: in-time pileup at bunch 0
    if (bunch_offset == 0) {
      if (!do_poisson_in_time_) {
        // fix it to the average
        n_events_overlay = static_cast<int>(poisson_mu_);
      } else if (do_poisson_in_time_ && !do_poisson_out_of_time_) {
        // then we haven't set this yet
        n_events_overlay = rndm_->Poisson(poisson_mu_);
      }

      // paticularly useful in the poisson fluctuated case
      event.getEventHeader().setIntParameter("in_time_pu", n_events_overlay);

      // the total number of events is nPU + 1 (it includes the sim event)
      // in any case, subtract the sim event from nOverlay
      n_events_overlay -= 1;

    }  // end if bunch_offset == 0

    float bunchtime_offset = bunch_spacing_ * bunch_offset;
    ldmx_log(debug) << "Will overlay " << n_events_overlay
                    << " events on the simulated one";

    for (int i_ev = 0; i_ev < n_events_overlay; i_ev++) {
      /** Go to next overlay event
       * This overlay file has been configured to loop back to the beginning
       * of the TTree when it reaches the end. This means nextEvent() will
       * only return false if an error is occurred or if the overlay file is
       * mis-configured.
       */
      if (!overlay_file_->nextEvent()) {
        ldmx_log(error) << "Couldn't read next overlay event!";
        return;
      }

      // a pileup event wide time offset to be applied to all its hits.
      float time_offset = rndm_time_->Gaus(time_mean_, time_sigma_);
      time_offset += bunchtime_offset;

      ldmx_log(trace) << "in overlay loop: overlaying event " << "which is "
                      << i_ev + 1 << " out of " << n_events_overlay
                      << "\n\thit time offset is " << time_offset << " ns"
                      << "\n\tbunch position offset is " << bunch_offset
                      << ", leading to a total time offset of "
                      << bunchtime_offset << " ns";

      /* ----------- first do the SimCalorimeterHits overlay ----------- */

      // again get the calo hits collections that we want to overlay
      for (uint i_coll = 0; i_coll < calo_collections_.size(); i_coll++) {
        // for now, Ecal and only Ecal uses contribs
        bool needs_contribs_added = false;
        if (strstr(calo_collections_[i_coll].c_str(), "Ecal")) {
          needs_contribs_added = true;
        }

        std::vector<ldmx::SimCalorimeterHit> overlay_hits =
            overlay_event_.getCollection<ldmx::SimCalorimeterHit>(
                calo_collections_[i_coll], overlay_passname_);

        ldmx_log(debug) << "in loop: size of overlay hits vector is "
                        << overlay_hits.size();

        std::string out_coll_name =
            calo_collections_[i_coll] + out_coll_postfix_;

        ldmx_log(trace) << "in loop: printing overlay event: ";

        for (ldmx::SimCalorimeterHit &overlay_hit : overlay_hits) {
          ldmx_log(trace) << overlay_hit;

          const float overlay_time = overlay_hit.getTime() + time_offset;
          overlay_hit.setTime(overlay_time);

          if (needs_contribs_added) {  // special treatment for (for now only)
                                       // ecal
            int overlay_hitID = overlay_hit.getID();
            if (hit_map.find(overlay_hitID) ==
                hit_map.end()) {  // there wasn't already a simhit in this id
              hit_map[overlay_hitID] = ldmx::SimCalorimeterHit();
              hit_map[overlay_hitID].setID(overlay_hitID);
              std::vector<float> hitPos = overlay_hit.getPosition();
              hit_map[overlay_hitID].setPosition(hitPos[0], hitPos[1],
                                                 hitPos[2]);
            }
            // add the overlay hit (as a) contrib
            // incidentID = -1000, trackID = -1000, pdgCode = 0  <-- these are
            // set in the header for now but could be parameters
            hit_map[overlay_hitID].addContrib(
                overlay_incident_id_, overlay_track_id_, overlay_pdg_code_,
                overlay_hit.getEdep(), overlay_time);
          }  // if add overlay as contribs
          else {
            calo_collection_map[out_coll_name].push_back(overlay_hit);

            ldmx_log(trace) << "Adding non-Ecal overlay hit to outhit vector "
                            << out_coll_name;
          }  // end else !needs_contribs_added
        }  // over overlay calo simhit collection

        if (!needs_contribs_added)
          ldmx_log(debug) << "Nhits in overlay collection " << out_coll_name
                          << ": " << calo_collection_map[out_coll_name].size();

      }  // over caloCollections

      /* ----------- now do simtracker hits overlay ----------- */

      // get the SimTrackerHit collections that we want to overlay
      for (const auto &coll : tracker_collections_) {
        auto overlay_tracker_hits{
            overlay_event_.getCollection<ldmx::SimTrackerHit>(
                coll, overlay_passname_)};

        ldmx_log(debug) << "in loop: size of overlay hits vector is "
                        << overlay_tracker_hits.size();

        std::string out_coll_name_tracker{coll + out_coll_postfix_};

        ldmx_log(trace) << "in loop: printing overlay event: ";

        for (auto &overlay_hit : overlay_tracker_hits) {
          auto overlay_time{overlay_hit.getTime() + time_offset};
          overlay_hit.setTime(overlay_time);
          tracker_collection_map[out_coll_name_tracker].push_back(overlay_hit);

          ldmx_log(trace) << overlay_hit;
          ldmx_log(trace) << "Adding tracker overlay hit to outhit vector "
                          << out_coll_name_tracker;
        }  // over overlay tracker simhit collection

        ldmx_log(debug) << "Nhits in overlay collection "
                        << out_coll_name_tracker << ": "
                        << tracker_collection_map[out_coll_name_tracker].size();

      }  // over trackerCollections
    }  // over overlay events
  }  // over bunches

  // after all events are done, the ecal hit_map is final and can be written
  // to the event output
  for (uint i_coll = 0; i_coll < calo_collections_.size(); i_coll++) {
    // loop through collection names to find the right collection name
    // add overlaid ecal hits as contribs/from hit_map rather than as copied
    // simhits
    if (strstr(calo_collections_[i_coll].c_str(), "Ecal")) {
      ldmx_log(trace) << "Hits in hit_map after overlay of "
                      << calo_collections_[i_coll] << "Overlay :";

      for (auto &map_hit : hit_map) {
        ldmx_log(trace) << map_hit.second;

        if (calo_collection_map.find(calo_collections_[i_coll] +
                                     out_coll_postfix_) ==
            calo_collection_map.end()) {
          ldmx_log(debug) << "Adding first hit from hit map as first outhit "
                             "vector to calo_collection_map";
          calo_collection_map[calo_collections_[i_coll] + out_coll_postfix_] = {
              map_hit.second};
        } else {
          calo_collection_map[calo_collections_[i_coll] + out_coll_postfix_]
              .push_back(map_hit.second);
        }
      }  // over hit_map
      break;  // for now we only have one hit_map: for Ecal. so no need
              // looking further after we got a match
    }  // isEcal
  }  // second loop over collections, to collect hits from hit_map

  // done collecting hits.

  // now we can write the calo collections to the event bus
  for (auto &[name, coll] : calo_collection_map) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";

    ldmx_log(trace) << "List of hits added: ";
    for (auto &hit : coll) {
      ldmx_log(trace) << hit;
    }
    event.add(name, coll);
  }

  // and now for the tracker hits
  for (auto &[name, coll] : tracker_collection_map) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";
    ldmx_log(trace) << "List of hits added: ";
    for (auto &hit : coll) {
      ldmx_log(trace) << hit;
    }
    event.add(name, coll);
  }
  return;
}  // end produce()

void OverlayProducer::onProcessStart() {
  // replace by this line once the corresponding tweak to EventFile is ready:
  overlay_file_ =
      std::make_unique<framework::EventFile>(params_, overlay_filename_, true);
  overlay_file_->setupEvent(&overlay_event_);
  // we update the iterator at the end of each event. so do this once here to
  // grab the first event in the processor
  // ldmx_log(trace) << "Used input file: "
  // << overlay_file_->getFileName() << " Got event info: " <<
  // overlay_file_->getEvent()->Print();

  return;
}  // end onProcessStart

}  // namespace recon
DECLARE_PRODUCER(recon::OverlayProducer)
