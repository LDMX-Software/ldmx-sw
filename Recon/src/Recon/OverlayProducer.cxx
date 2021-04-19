#include "Recon/OverlayProducer.h"
#include "Framework/RandomNumberSeedService.h"

#include "SimCore/Event/SimTrackerHit.h"
#include "SimCore/Event/SimCalorimeterHit.h"

namespace recon {

void OverlayProducer::configure(framework::config::Parameters &parameters) {
  params_ = parameters;

  ldmx_log(debug) << "Running configure() ";

  // name of file containing events to be overlaid, and a list of collections to
  // overlay
  overlayFileName_ = parameters.getParameter<std::string>("overlayFileName");
  caloCollections_ = parameters.getParameter<std::vector<std::string>>(
      "overlayCaloHitCollections");
  trackerCollections_ = parameters.getParameter<std::vector<std::string>>(
      "overlayTrackerHitCollections");
  simPassName_ = parameters.getParameter<std::string>("passName");
  overlayPassName_ = parameters.getParameter<std::string>("overlayPassName");
  // overlay specifics:
  poissonMu_ = parameters.getParameter<double>("totalNumberOfInteractions");
  doPoissonIT_ = parameters.getParameter<bool>("doPoissonIntime");
  doPoissonOOT_ = parameters.getParameter<bool>("doPoissonOutoftime");
  timeSigma_ = parameters.getParameter<double>("timeSpread");
  timeMean_ = parameters.getParameter<double>("timeMean");
  nEarlier_ = parameters.getParameter<int>("nEarlierBunchesToSample");
  nLater_ = parameters.getParameter<int>("nLaterBunchesToSample");
  bunchSpacing_ = parameters.getParameter<double>("bunchSpacing");
  verbosity_ = parameters.getParameter<int>("verbosity");

  /// Print the parameters actually set. Helpful in case of typos.
  if (verbosity_) {
    ldmx_log(info) << "Got parameters \n \t overlayFileName = "
                   << overlayFileName_
                   << "\n\t sim pass name = " << simPassName_
                   << "\n\t overlay pass name = " << overlayPassName_
                   << "\n\t overlayCaloHitCollections = ";
    for (const auto &coll : caloCollections_) ldmx_log(info) << coll << "; ";

    ldmx_log(info) << "\n\t overlayTrackerHitCollections = ";
    for (const std::string &coll : trackerCollections_)
      ldmx_log(info) << coll << "; ";

    ldmx_log(info) << "\n\t numberOverlaidInteractions = " << poissonMu_
                   << "\n\t nEarlierBunchesToSample = " << nEarlier_
                   << "\n\t nLaterBunchesToSample = " << nLater_
                   << "\n\t bunchSpacing = " << bunchSpacing_
                   << "\n\t doPoissonIntime = " << doPoissonIT_
                   << "\n\t doPoissonOutoftime = " << doPoissonOOT_
                   << "\n\t timeSpread = " << timeSigma_
                   << "\n\t timeMean = " << timeMean_
                   << "\n\t verbosity = " << verbosity_;
  }
  return;
}

void OverlayProducer::onNewRun(const ldmx::RunHeader &) {
  /// set up random seeds
  if (rndm_.get() == nullptr) {
    // not been seeded yet, get it from RNSS
    const auto &rnss = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    rndm_ = std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndm"));
  }

  int start_event = rndm_->Uniform(20., 1e4);
  // EventFile::skipToEvent handles actual number of events in file
  int evNb = overlayFile_->skipToEvent(start_event);
  if (evNb < 0) {
    EXCEPTION_RAISE("BadRead", "Couldn't read to starting offset.");
  }
  overlayEvent_.getEventHeader().setEventNumber(evNb);
  ldmx_log(info) << "Starting overlay process with pileup event number " << evNb
                 << " (random event number picked was " << start_event << ").";
}

void OverlayProducer::produce(framework::Event &event) {
  // event is the incoming, simulated event/"hard" process
  // overlayEvent_ is the overlay producer's own event.
  if (verbosity_ > 1) {
    ldmx_log(info) << "produce() starts on simulation event "
                   << event.getEventHeader().getEventNumber();
  }

  if (rndmTime_.get() == nullptr) {
    // not been seeded yet, get it from RNSS
    const auto &rnss = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    rndmTime_ =
        std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndmTime"));
  }

  // using nextEvent to loop, we need to loop over overlay events and in an
  // inner loop, loop over collections, and store them. after all pileup events
  // have been added, the vector of collections is iterated over and added to
  // the event bus.
  std::map<std::string, std::vector<ldmx::SimCalorimeterHit>> caloCollectionMap;
  std::map<std::string, std::vector<ldmx::SimTrackerHit>> trackerCollectionMap;
  std::map<int, ldmx::SimCalorimeterHit> hitMap;

  // start by copying over all the collections from the sim event

  /* ----------- first do the SimCalorimeterHits ----------- */

  // get the calo hits collections that we want to overlay, by looping over
  // the list of collections passed to the producer : caloCollections_
  for (const auto &collName : caloCollections_) {
    // for now, Ecal and only Ecal uses contribs instead of multiple
    // SimHitsCalo per channel, meaning, it requires special treatment
    auto needsContribsAdded{collName.find("Ecal") != std::string::npos ? true
                                                                       : false};

    // start out by just copying the sim hits, unaltered.
    auto simHitsCalo =
        event.getCollection<ldmx::SimCalorimeterHit>(collName, simPassName_);
    // but don't copy ecal hits immediately: for them, wait until overlay
    // contribs have been added. then add everything through the hitmap
    if (!needsContribsAdded) {
      caloCollectionMap[collName + "Overlay"] = simHitsCalo;
    }

    if (verbosity_ > 2) {
      ldmx_log(debug) << "in loop: start of collection " << collName
                      << "in loop: printing current sim event: ";
    }
    ldmx_log(debug) << "in loop: size of sim hits vector " << collName << " is "
                    << simHitsCalo.size();

    // we don't need to touch the hard process sim hits, really... but we
    // might need the simhits in the hit map.
    if (needsContribsAdded || verbosity_ > 2) {
      for (const ldmx::SimCalorimeterHit &simHit : simHitsCalo) {
        if (verbosity_ > 2) simHit.Print();

        if (needsContribsAdded) {
          // this copies the hit, its ID and its coordinates directly
          hitMap[simHit.getID()] = simHit;
        }

      }  // over calo simhit collection
    }    // if needContribs or very verbose

  }  // over calo collections for sim event

  /* ----------- now do the same with SimTrackerHits! ----------- */

  // get the SimTrackerHit collections that we want to overlay, by looping
  // over the list of collections passed to the producer : trackerCollections_
  for (const auto &collName : trackerCollections_) {
    auto simHitsTracker =
        event.getCollection<ldmx::SimTrackerHit>(collName, simPassName_);
    trackerCollectionMap[collName + "Overlay"] = simHitsTracker;

    // the rest is printouts for debugging
    ldmx_log(debug) << "in loop: size of sim hits vector " << collName << " is "
                    << simHitsTracker.size();

    if (verbosity_ > 2) {
      ldmx_log(debug) << "in loop: start of collection " << collName
                      << "in loop: printing current sim event: ";

      for (const ldmx::SimTrackerHit &simHit : simHitsTracker) simHit.Print();
    }  // if high verbosity
  }    // over tracker collections for sim event

  /* ----------- now do the pileup overlay ----------- */

  // we could shift these by a random number, effectively placing the
  // sim event at random positions in the interval, preserving the
  // overall interval length
  // int simBunch= (int)rndmTime_->Uniform(
  //				   -(nEarlier_+1) , nLater_+1);  // +1 to get
  // inclusive interval
  int startBunch = -nEarlier_;
  int endBunch = nLater_;

  // TODO -- figure out if we should also randomly shift the time of the sim
  // event (likely only needed if time bias gets picked up by BDT or ML by way
  // of pulse behaviour)
  for (int bunchOffset{startBunch}; bunchOffset <= endBunch; bunchOffset++) {
    // sample a poisson distribution, or use mu as fixed number of overlay
    // events
    int nEvsOverlay =
        doPoissonOOT_ ? (int)rndm_->Poisson(poissonMu_) : (int)poissonMu_;

    // special case: in-time pileup at bunch 0
    if (bunchOffset == 0) {
      if (!doPoissonIT_)
        nEvsOverlay = (int)poissonMu_;          // fix it to the average
      else if (doPoissonIT_ && !doPoissonOOT_)  // then we haven't set this yet
        nEvsOverlay = (int)rndm_->Poisson(poissonMu_);

      // paticularly useful in the poisson fluctuated case
      event.getEventHeader().setIntParameter("inTimePU", nEvsOverlay);

      // the total number of events is nPU + 1 (it includes the sim event)
      nEvsOverlay -= 1;  // in any case, subtract the sim event from nOverlay
      if (verbosity_ > 2) {
        ldmx_log(debug) << "will overlay " << nEvsOverlay
                        << " events on the simulated one";
      }
    }

    // get event wherever nextEvent()  left us
    if (!&overlayEvent_) {
      ldmx_log(error) << "No overlay event!";
      return;
    }

    float bunchTimeOffset = bunchSpacing_ * bunchOffset;

    for (int iEv = 0; iEv < nEvsOverlay; iEv++) {
      // a pileup event wide time offset to be applied to all its hits.
      float timeOffset = rndmTime_->Gaus(timeMean_, timeSigma_);
      timeOffset += bunchTimeOffset;

      if (verbosity_ > 2) {
        ldmx_log(debug) << "in overlay loop: overlaying event "
                        << overlayEvent_.getEventHeader().getEventNumber()
                        << "which is " << iEv + 1 << " out of " << nEvsOverlay
                        << "\n\thit time offset is " << timeOffset << " ns"
                        << "\n\tbunch position offset is " << bunchOffset
                        << ", leading to a total time offset of "
                        << bunchTimeOffset << " ns";
      }

      /* ----------- first do the SimCalorimeterHits overlay ----------- */

      // again get the calo hits collections that we want to overlay
      for (uint iColl = 0; iColl < caloCollections_.size(); iColl++) {
        // for now, Ecal and only Ecal uses contribs
        bool needsContribsAdded = false;
        if (strstr(caloCollections_[iColl].c_str(), "Ecal"))
          needsContribsAdded = true;

        std::vector<ldmx::SimCalorimeterHit> overlayHits =
            overlayEvent_.getCollection<ldmx::SimCalorimeterHit>(
                caloCollections_[iColl], overlayPassName_);

        ldmx_log(debug) << "in loop: size of overlay hits vector is "
                        << overlayHits.size();

        std::string outCollName = caloCollections_[iColl] + "Overlay";

        if (verbosity_ > 2) {
          ldmx_log(debug) << "in loop: printing overlay event: ";
        }

        for (ldmx::SimCalorimeterHit &overlayHit : overlayHits) {
          if (verbosity_ > 2) overlayHit.Print();

          const float overlayTime = overlayHit.getTime() + timeOffset;
          overlayHit.setTime(overlayTime);

          if (needsContribsAdded) {  // special treatment for (for now only)
                                     // ecal
            int overlayHitID = overlayHit.getID();
            if (hitMap.find(overlayHitID) ==
                hitMap.end()) {  // there wasn't already a simhit in this id
              hitMap[overlayHitID] = ldmx::SimCalorimeterHit();
              hitMap[overlayHitID].setID(overlayHitID);
              std::vector<float> hitPos = overlayHit.getPosition();
              hitMap[overlayHitID].setPosition(hitPos[0], hitPos[1], hitPos[2]);
            }
            // add the overlay hit (as a) contrib
            // incidentID = -1000, trackID = -1000, pdgCode = 0  <-- these are
            // set in the header for now but could be parameters
            hitMap[overlayHitID].addContrib(overlayIncidentID_, overlayTrackID_,
                                            overlayPdgCode_,
                                            overlayHit.getEdep(), overlayTime);
          }  // if add overlay as contribs
          else {
            caloCollectionMap[outCollName].push_back(overlayHit);
            if (verbosity_ > 2)
              ldmx_log(debug) << "Adding non-Ecal overlay hit to outhit vector "
                              << outCollName;
          }
        }  // over overlay calo simhit collection

        if (!needsContribsAdded)
          ldmx_log(debug) << "Nhits in overlay collection " << outCollName
                          << ": " << caloCollectionMap[outCollName].size();

      }  // over caloCollections

      /* ----------- now do simtracker hits overlay ----------- */

      // get the SimTrackerHit collections that we want to overlay
      for (const auto &coll : trackerCollections_) {
        auto overlayTrackerHits{
            overlayEvent_.getCollection<ldmx::SimTrackerHit>(coll,
                                                             overlayPassName_)};

        ldmx_log(debug) << "in loop: size of overlay hits vector is "
                        << overlayTrackerHits.size();

        auto outCollName{coll + "Overlay"};

        if (verbosity_ > 2) {
          ldmx_log(debug) << "in loop: printing overlay event: ";
        }

        for (auto &overlayHit : overlayTrackerHits) {
          auto overlayTime{overlayHit.getTime() + timeOffset};
          overlayHit.setTime(overlayTime);
          trackerCollectionMap[outCollName].push_back(overlayHit);

          if (verbosity_ > 2) {
            overlayHit.Print();
            ldmx_log(debug) << "Adding tracker overlay hit to outhit vector "
                            << outCollName;
          }  // verbose
        }    // over overlay tracker simhit collection

        ldmx_log(debug) << "Nhits in overlay collection " << outCollName << ": "
                        << trackerCollectionMap[outCollName].size();

      }  // over trackerCollections

      // update the event number. here, possibly the event counter gets reset to
      // 0, if we hit the end of the pileup event tree.
      if (!overlayFile_->nextEvent()) {
        ldmx_log(error) << "At sim event "
                        << event.getEventHeader().getEventNumber()
                        << ": couldn't read next overlay event!";
        return;
      }
    }  // over overlay events
  }    // over bunches

  // after all events are done, the ecal hitmap is final and can be written to
  // the event output
  for (uint iColl = 0; iColl < caloCollections_.size(); iColl++) {
    // loop through collection names to find the right collection name
    // add overlaid ecal hits as contribs/from hitmap rather than as copied
    // simhits
    if (strstr(caloCollections_[iColl].c_str(), "Ecal")) {
      if (verbosity_ > 2)
        ldmx_log(debug) << "Hits in hitmap after overlay of "
                        << caloCollections_[iColl] << "Overlay :";

      for (auto &mapHit : hitMap) {
        if (verbosity_ > 2) mapHit.second.Print();

        if (caloCollectionMap.find(caloCollections_[iColl] + "Overlay") ==
            caloCollectionMap.end()) {
          ldmx_log(debug) << "Adding first hit from hit map as first outhit "
                             "vector to caloCollectionMap";
          caloCollectionMap[caloCollections_[iColl] + "Overlay"] = {
              mapHit.second};
        } else
          caloCollectionMap[caloCollections_[iColl] + "Overlay"].push_back(
              mapHit.second);
      }
      break;  // for now we only have one hitMap: for Ecal. so no need looking
              // further after we got a match
    }         // isEcal
  }           // second loop over collections, to collect hits from hitmap

  // done collecting hits.

  // this should be added to the sim file, so to "event"
  // once for each hit type
  for (auto &[name, coll] : caloCollectionMap) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";
    if (verbosity_ > 2) {
      ldmx_log(debug) << "List of hits added: ";
      for (auto &hit : coll) hit.Print();
    }
    event.add(name, coll);
  }
  for (auto &[name, coll] : trackerCollectionMap) {
    ldmx_log(debug) << "Writing " << name << " to event bus.";
    if (verbosity_ > 2) {
      ldmx_log(debug) << "List of hits added: ";
      for (auto &hit : coll) hit.Print();
    }
    event.add(name, coll);
  }
  return;
}

void OverlayProducer::onProcessStart() {
  if (verbosity_ > 2) {
    ldmx_log(debug) << "onProcessStart() ";
  }

  // replace by this line once the corresponding tweak to EventFile is ready:
  overlayFile_ =
      std::make_unique<framework::EventFile>(params_, overlayFileName_, true);
  overlayFile_->setupEvent(&overlayEvent_);
  // we update the iterator at the end of each event. so do this once here to
  // grab the first event in the processor

  if (verbosity_ > 2) {
    ldmx_log(debug) << "onProcessStart () successful. Used input file: "
                    << overlayFile_->getFileName();
    ldmx_log(debug) << "onProcessStart () successful. Got event info: ";
    overlayFile_->getEvent()->Print();
  }

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, OverlayProducer)
