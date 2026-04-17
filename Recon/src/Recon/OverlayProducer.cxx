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
  particle_collections_ =
      parameters.get<std::vector<std::string>>("particle_collections");
  contrib_collections_ =
      parameters.get<std::vector<std::string>>("contrib_collections");
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
  track_id_encoding_ = unsigned(parameters.get<int>("track_id_encoding", 0));

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

  ldmx_log(debug) << "\n\t overlayParticleCollections = ";
  for (const std::string &coll : particle_collections_) {
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

  // given how the contrib collection specification is setup, it's possible for
  // spelling errors to cause doom here, so we want to flag those
  for (auto coll_name : contrib_collections_) {
    if (std::find(calo_collections_.begin(), calo_collections_.end(),
                  coll_name) == calo_collections_.end()) {
      EXCEPTION_RAISE("CollNameMismatch",
                      "The contrib-using collection " + coll_name +
                          " does not match any SimCalorimeterHit collection in "
                          "'calo_collections_'! Please check your spelling");
    }
  }

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
  std::map<std::string, std::map<int, ldmx::SimParticle>>
      particle_collection_map;
  std::map<std::string, std::map<int, ldmx::SimCalorimeterHit>> hit_map;

  // start by copying over all the collections from the sim event

  /* ----------- first do the SimCalorimeterHits ----------- */

  // get the calo hits_ collections that we want to overlay, by looping over
  // the list of collections passed to the producer : calo_collections_
  for (const auto &coll_name : calo_collections_) {
    // search for the collection name in the list of collections that
    // need contribs to be added, contrib_collections_
    bool needs_contribs_added{std::find(contrib_collections_.begin(),
                                        contrib_collections_.end(),
                                        coll_name) != contrib_collections_.end()
                                  ? true
                                  : false};

    // start out by just copying the sim hits, unaltered.
    auto simhits_calo =
        event.getCollection<ldmx::SimCalorimeterHit>(coll_name, sim_passname_);
    // but don't copy contrib using hits immediately: for them, wait until
    // overlay contribs have been added. then add everything through the hit_map
    if (!needs_contribs_added) {
      calo_collection_map[coll_name + out_coll_postfix_] = simhits_calo;
      // after copying, perform track ID encodings;
      // the main sample gets the event index 0 by default
      for (auto &simhit : calo_collection_map[coll_name + out_coll_postfix_]) {
        simhit.encodeTracks(
            [this](int id, unsigned enc, unsigned idx) {
              return encodeTrack(id, enc, idx);
            },
            track_id_encoding_, (unsigned)0);
      }
    }

    ldmx_log(debug) << "in loop: start of collection " << coll_name
                    << "in loop: printing current sim event: ";
    ldmx_log(debug) << "in loop: size of sim hits_ vector " << coll_name
                    << " is " << simhits_calo.size();

    // we don't need to touch the hard process sim hits_, really... but we
    // might need the simhits in the hit map.
    if (needs_contribs_added) {
      ldmx_log(trace) << "Collection " << coll_name << " needs contribs added";
      for (const ldmx::SimCalorimeterHit &simhit : simhits_calo) {
        ldmx_log(trace) << simhit;
        // this copies the hit, its ID and its coordinates directly
        hit_map[coll_name + out_coll_postfix_][simhit.getID()] = simhit;
        // now encode track IDs on copied object
        hit_map[coll_name + out_coll_postfix_][simhit.getID()].encodeTracks(
            [this](int id, unsigned enc, unsigned idx) {
              return encodeTrack(id, enc, idx);
            },
            track_id_encoding_, (unsigned)0);
      }  // over calo simhit collection
    }  // if needContribs

  }  // over calo collections for sim event

  /* ----------- then do the same with SimTrackerHits! ----------- */

  // get the SimTrackerHit collections that we want to overlay, by looping
  // over the list of collections passed to the producer : tracker_collections_
  for (const auto &coll_name : tracker_collections_) {
    auto simhits_tracker =
        event.getCollection<ldmx::SimTrackerHit>(coll_name, sim_passname_);
    tracker_collection_map[coll_name + out_coll_postfix_] = simhits_tracker;
    // after copying, perform track ID encoding
    for (auto &simhit : tracker_collection_map[coll_name + out_coll_postfix_]) {
      int encoded_track_id =
          encodeTrack(simhit.getTrackID(), track_id_encoding_, (unsigned)0);
      simhit.setTrackID(encoded_track_id);
    }

    // the rest is printouts for debugging
    ldmx_log(debug) << "in loop: size of sim hits_ vector " << coll_name
                    << " is " << simhits_tracker.size();

    ldmx_log(debug) << "in loop: start of collection " << coll_name
                    << "in loop: printing current sim event: ";

    for (const ldmx::SimTrackerHit &simhit : simhits_tracker) {
      ldmx_log(trace) << simhit;
    }
  }  // over tracker collections for sim event

  /* ----------- and finish up with SimParticles ----------- */

  // get the SimParticle collections that we want to overlay, by looping
  // over the list of collections passed to the producer : particle_collections_
  for (const auto &coll_name : particle_collections_) {
    auto sim_particles =
        event.getMap<int, ldmx::SimParticle>(coll_name, sim_passname_);
    auto &output_map = particle_collection_map[coll_name + out_coll_postfix_];

    ldmx_log(debug) << "in loop: size of sim particles map " << coll_name
                    << " is " << sim_particles.size();

    ldmx_log(debug) << "in loop: start of collection " << coll_name
                    << "in loop: printing current sim event: ";

    // need to copy SimParticles individually because of std::map key
    // immutability
    for (auto &[track_id, particle] : sim_particles) {
      int encoded_track_id =
          encodeTrack(track_id, track_id_encoding_, (unsigned)0);
      output_map[encoded_track_id] = particle;
      output_map[encoded_track_id].encodeTracks(
          [this](int id, unsigned enc, unsigned idx) {
            return encodeTrack(id, enc, idx);
          },
          track_id_encoding_, (unsigned)0);
      ldmx_log(trace) << particle;
    }
  }  // over particle collections for sim event

  /* ----------- now do the overlay ----------- */

  // we could shift these by a random number, effectively placing the
  // sim event at random positions in the interval, preserving the
  // overall interval length
  // int simBunch=  static_cast<int>(rndm_time_->Uniform(
  //				   -(n_earlier_+1) , n_later_+1));  // +1 to get
  // inclusive interval
  int start_bunch = -n_earlier_;
  int end_bunch = n_later_;

  // this is the event index number assigned to overlay sample hits and
  // particles for track ID encoding
  unsigned overlay_event_index = 1;

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

      // a pileup event wide time offset to be applied to all its hits_.
      float time_offset = rndm_time_->Gaus(time_mean_, time_sigma_);
      time_offset += bunchtime_offset;

      ldmx_log(trace) << "in overlay loop: overlaying event " << "which is "
                      << i_ev + 1 << " out of " << n_events_overlay
                      << "\n\thit time offset is " << time_offset << " ns"
                      << "\n\tbunch position offset is " << bunch_offset
                      << ", leading to a total time offset of "
                      << bunchtime_offset << " ns";

      /* ----------- first do the SimCalorimeterHits overlay ----------- */

      // again get the calo hits_ collections that we want to overlay
      for (uint i_coll = 0; i_coll < calo_collections_.size(); i_coll++) {
        // search for the collection name in the list of collections that
        // need contribs to be added, contrib_collections_
        bool needs_contribs_added{
            std::find(contrib_collections_.begin(), contrib_collections_.end(),
                      calo_collections_[i_coll]) != contrib_collections_.end()
                ? true
                : false};

        std::vector<ldmx::SimCalorimeterHit> overlay_hits =
            overlay_event_.getCollection<ldmx::SimCalorimeterHit>(
                calo_collections_[i_coll], overlay_passname_);

        ldmx_log(debug) << "in loop: size of overlay hits_ vector is "
                        << overlay_hits.size();

        std::string out_coll_name =
            calo_collections_[i_coll] + out_coll_postfix_;

        ldmx_log(trace) << "in loop: printing overlay event: ";

        for (ldmx::SimCalorimeterHit &overlay_hit : overlay_hits) {
          ldmx_log(trace) << overlay_hit;

          // update relevant parameters with overlay modifications
          overlay_hit.setTime(overlay_hit.getTime() + time_offset);
          overlay_hit.setPreStepTime(overlay_hit.getPreStepTime() +
                                     time_offset);
          overlay_hit.setPostStepTime(overlay_hit.getPostStepTime() +
                                      time_offset);
          overlay_hit.encodeTracks(
              [this](int id, unsigned enc, unsigned idx) {
                return encodeTrack(id, enc, idx);
              },
              track_id_encoding_, overlay_event_index);

          if (needs_contribs_added) {  // special treatment for (for now only)
                                       // ecal
            auto &this_coll_hit_map{
                hit_map[calo_collections_[i_coll] + out_coll_postfix_]};
            int overlay_hit_id = overlay_hit.getID();
            if (this_coll_hit_map.find(overlay_hit_id) ==
                this_coll_hit_map
                    .end()) {  // there wasn't already a simhit in this id
              ldmx_log(trace)
                  << "No existing simhit found for ID " << overlay_hit_id
                  << "; copying overlay hit to output collection";
              auto &output_hit = this_coll_hit_map[overlay_hit_id];
              output_hit = overlay_hit;  // copy hit

              // we need to do some backflips to get the contribs correct later
              auto id = output_hit.getID();
              auto pos = output_hit.getPosition();
              auto time = output_hit.getTime();
              output_hit.clear();  // with the below adjustments, this will only
                                   // clear contribs from the output_hit
              output_hit.setID(id);
              output_hit.setPosition(pos[0], pos[1], pos[2]);
              output_hit.setTime(time);
            }  // if overlay_hit_id not present

            // add the overlay hit contribs to existing hit,
            // incrementing track IDs and timestamp as needed
            int n_contribs = overlay_hit.getNumberOfContribs();
            ldmx_log(trace)
                << "Copying and reindexing " << n_contribs
                << " contributors to the sim hit for ID " << overlay_hit_id;
            for (int i = 0; i < n_contribs; i++) {
              ldmx::SimCalorimeterHit::Contrib contrib{
                  overlay_hit.getContrib(i)};
              // tracks were encoded in-place above, so we only need to worry
              // about times
              this_coll_hit_map[overlay_hit_id].addContrib(
                  contrib.incident_id_, contrib.track_id_, contrib.pdg_code_,
                  contrib.edep_, contrib.time_ + time_offset);
            }  // loop over contribs in overlay_hit
            ldmx_log(trace)
                << "There are now "
                << this_coll_hit_map[overlay_hit_id].getNumberOfContribs()
                << " total contributors in the output collection";
          }  // if add overlay as contribs
          else {
            // no need to change contrib timestamps here
            calo_collection_map[out_coll_name].push_back(overlay_hit);

            ldmx_log(trace) << "Adding non-Ecal overlay hit to outhit vector "
                            << out_coll_name;
          }  // end else !needs_contribs_added
        }  // over overlay calo simhit collection

        if (!needs_contribs_added)
          ldmx_log(debug) << "Nhits in overlay collection " << out_coll_name
                          << ": " << calo_collection_map[out_coll_name].size();

      }  // over calo_collections_

      /* ----------- now do simtracker hits_ overlay ----------- */

      // loop over the SimTrackerHit collections that we want to overlay
      for (const auto &coll : tracker_collections_) {
        auto overlay_tracker_hits{
            overlay_event_.getCollection<ldmx::SimTrackerHit>(
                coll, overlay_passname_)};

        ldmx_log(debug) << "in loop: size of overlay hits_ vector is "
                        << overlay_tracker_hits.size();

        std::string out_coll_name_tracker{coll + out_coll_postfix_};

        ldmx_log(trace) << "in loop: printing overlay event: ";

        for (auto &overlay_hit : overlay_tracker_hits) {
          auto overlay_time{overlay_hit.getTime() + time_offset};
          overlay_hit.setTime(overlay_time);
          auto overlay_track_id{encodeTrack(overlay_hit.getTrackID(),
                                            track_id_encoding_,
                                            overlay_event_index)};
          overlay_hit.setTrackID(overlay_track_id);

          ldmx_log(trace) << overlay_hit;
          ldmx_log(trace) << "Adding tracker overlay hit to outhit vector "
                          << out_coll_name_tracker;

          tracker_collection_map[out_coll_name_tracker].push_back(overlay_hit);
        }  // over overlay tracker simhit collection

        ldmx_log(debug) << "Nhits in overlay collection "
                        << out_coll_name_tracker << ": "
                        << tracker_collection_map[out_coll_name_tracker].size();

      }  // over tracker_collections_

      /* ----------- finally do SimParticles overlay ----------- */
      for (const auto &coll : particle_collections_) {
        auto overlay_particles{overlay_event_.getMap<int, ldmx::SimParticle>(
            coll, overlay_passname_)};

        ldmx_log(debug) << "in loop: size of overlay particles map is "
                        << overlay_particles.size();

        std::string out_coll_name_particles{coll + out_coll_postfix_};
        auto &output_map = particle_collection_map[out_coll_name_particles];

        ldmx_log(trace) << "in loop: printing overlay event: ";

        for (auto &[track_id, particle] : overlay_particles) {
          int new_track_id =
              encodeTrack(track_id, track_id_encoding_, overlay_event_index);

          output_map[new_track_id] = particle;
          output_map[new_track_id].encodeTracks(
              [this](int id, unsigned enc, unsigned idx) {
                return encodeTrack(id, enc, idx);
              },
              track_id_encoding_, overlay_event_index);
          output_map[new_track_id].setTime(particle.getTime() + time_offset);

          ldmx_log(trace) << "Track ID: " << new_track_id << " --- "
                          << output_map[new_track_id];
          ldmx_log(trace) << "Adding sim particle to output map "
                          << out_coll_name_particles;
        }  // over overlay sim particles collection
      }  // over particle_collections_

    }  // over overlay events
  }  // over bunches

  // after all events are done, the contrib-using collections' hit_maps are
  // final and can be written to the event output
  for (uint i_coll = 0; i_coll < contrib_collections_.size(); i_coll++) {
    // for each SimCalorimeterHit collection that uses contribs, add overlaid
    // hits_ as contribs/from hit_map rather than as copied simhits
    ldmx_log(trace) << "Hits in hit_map after overlay of "
                    << contrib_collections_[i_coll] << "Overlay :";

    for (auto &map_hit :
         hit_map[contrib_collections_[i_coll] + out_coll_postfix_]) {
      ldmx_log(trace) << map_hit.second;

      if (calo_collection_map.find(contrib_collections_[i_coll] +
                                   out_coll_postfix_) ==
          calo_collection_map.end()) {
        ldmx_log(debug) << "Adding first hit from hit map as first outhit "
                           "vector to calo_collection_map";
        calo_collection_map[contrib_collections_[i_coll] + out_coll_postfix_] =
            {map_hit.second};
      } else {
        calo_collection_map[contrib_collections_[i_coll] + out_coll_postfix_]
            .push_back(map_hit.second);
      }
    }  // over hit_map
  }  // second loop over contrib using collections, to collect hits_ from
     // hit_map

  // done collecting hits_.

  // now we can write the calo collections to the event bus
  for (auto &[name, coll] : calo_collection_map) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";

    ldmx_log(trace) << "List of hits_ added: ";
    for (auto &hit : coll) {
      ldmx_log(trace) << hit;
    }
    event.add(name, coll);
  }

  // and now for the tracker hits_
  for (auto &[name, coll] : tracker_collection_map) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";
    ldmx_log(trace) << "List of hits_ added: ";
    for (auto &hit : coll) {
      ldmx_log(trace) << hit;
    }
    event.add(name, coll);
  }

  // and finally for sim particles
  for (auto &[name, coll] : particle_collection_map) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";
    ldmx_log(trace) << "List of particles added: ";
    for (auto &[track_id, particle] : coll) {
      ldmx_log(trace) << "Track ID: " << track_id << " --- " << particle;
    }
    event.add(name, coll);
  }

  return;
}  // end produce()

int OverlayProducer::encodeTrack(int track_id,
                                 const unsigned int encoding_version,
                                 const unsigned int event_index) {
  ldmx_log(trace) << "Encoding track ID " << track_id
                  << " according to version " << encoding_version;
  int encoded_id;

  // check if the encoding_version will overflow into the int sign bit
  if (encoding_version > 15) {
    EXCEPTION_RAISE("TrackEncodingError",
                    "Track ID encoding version " +
                        std::to_string(encoding_version) +
                        " has exceeded version maximum value of 15");
  }
  if (track_id < 0) {
    // if track_id < 0 it's probably a default nonsense value -1;
    // encoding at present DOES NOT WORK for negative values because C++ uses
    // twos complement encoding for negative numbers;
    // thus when decoding you should first check the first bit, and
    // set it to a nonsense -1 if the number is negative.
    // Right now (2026-03-07) the only place where track IDs are -1 are the
    // origin ids in the SimCalorimeterHit contribs, which aren't used anyways
    // and present no risk for overwriting data for duplicate track IDs, so we
    // can leave this alone
    ldmx_log(trace) << "Track ID has value " << track_id
                    << " < 0; no encoding will be applied (this is expected "
                       "for origin IDs in SimCalorimeterHits)";
    return track_id;
  }

  // encode Track ID according to provided version
  switch (encoding_version) {
    case (unsigned)0: {  // need braces for variable init scope control
      ldmx_log(trace) << "No encoding applied";
      encoded_id = track_id;
      break;
    }
    case (unsigned)1: {
      // create bitwise masks for encoding from version number and event index
      unsigned version_mask = (encoding_version << 27);
      unsigned index_mask = (event_index << 24);

      // encode track ID using bitwise OR
      encoded_id = track_id | version_mask | index_mask;
      ldmx_log(trace) << "Encoding successful! encoded_id = " << encoded_id
                      << ", with bit representation "
                      << std::bitset<32>(encoded_id);
      break;
    }
    default: {  // version number has no associated encoding scheme
      ldmx_log(warn) << "Track ID encoding version " << encoding_version
                     << " has no definition! No encoding will be applied";
      encoded_id = track_id;
    }
  }

  return encoded_id;
}

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
