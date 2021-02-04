#include "Recon/OverlayProducer.h"
#include "Framework/RandomNumberSeedService.h"

namespace recon {

void OverlayProducer::configure(framework::config::Parameters &parameters) {
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
  doPoisson_ = parameters.getParameter<bool>("doPoisson");
  timeSigma_ = parameters.getParameter<double>("timeSpread");
  timeMean_ = parameters.getParameter<double>("timeMean");
  nBunchesToSample_ = parameters.getParameter<int>("nBunchesToSample");
  bunchSpacing_ = parameters.getParameter<double>("bunchSpacing");
  verbosity_ = parameters.getParameter<int>("verbosity");

  /// Print the parameters actually set. Helpful in case of typos.
  if (verbosity_) {
    ldmx_log(info) << "Got parameters \n \t overlayFileName = "
                   << overlayFileName_
                   << "\n\t sim pass name = " << simPassName_
                   << "\n\t overlay pass name = " << overlayPassName_
                   << "\n\t overlayCaloHitCollections = ";
    for (const std::string &coll : caloCollections_)
      ldmx_log(info) << coll << "; ";

    ldmx_log(info) << "\n\t overlayTrackerHitCollections = ";
    for (const std::string &coll : trackerCollections_)
      ldmx_log(info) << coll << "; ";

    ldmx_log(info) << "\n\t numberOverlaidInteractions = " << poissonMu_
                   << "\n\t doPoisson = " << doPoisson_
                   << "\n\t timeSpread = " << timeSigma_
                   << "\n\t timeMean = " << timeMean_
                   << "\n\t verbosity = " << verbosity_;
  }
  return;
}

void OverlayProducer::produce(framework::Event &event) {
  // event is the incoming, simulated event/"hard" process
  // overlayEvent_ is the overlay producer's own event.
  if (verbosity_ > 1) {
    ldmx_log(info) << "produce() starts on simulation event "
                   << event.getEventHeader().getEventNumber();
  }

  /// set up random seeds
  if (rndm_.get() == nullptr) {
    // not been seeded yet, get it from RNSS
    const auto &rnss = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    rndm_ = std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndm"));
  }
  if (rndmTime_.get() == nullptr) {
    // not been seeded yet, get it from RNSS
    const auto &rnss = getCondition<framework::RandomNumberSeedService>(
        framework::RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
    rndmTime_ =
        std::make_unique<TRandom2>(rnss.getSeed("OverlayProducer::rndmTime"));
  }

  // sample a poisson distribution, or use a deterministic number of overlay
  // events
  int nEvsOverlay =
      doPoisson_ ? (int)rndm_->Poisson(poissonMu_) : (int)poissonMu_;
  // the poisson samples the total number of events, which is nOverlay + 1
  // (since it includes the sim event)
  nEvsOverlay -= 1;  // now subtract the sim event from the poisson mu

  if (verbosity_ > 2) {
    ldmx_log(debug) << "will overlay " << nEvsOverlay
                    << " events on the simulated one";
  }

  // get event wherever nextEvent()  left us
  if (!&overlayEvent_) {
    ldmx_log(error) << "No overlay event!";
    return;
  }

  if (verbosity_ > 2) {
    ldmx_log(debug) << "starting from overlay event "
                    << overlayEvent_.getEventHeader().getEventNumber();
  }

  // using nextEvent to loop, we need to loop over overlay events and in an
  // inner loop, loop over collections, and store them. after all pileup events
  // have been added, the vector of collections is iterated over and added to
  // the event bus.
  std::map<std::string, std::vector<ldmx::SimCalorimeterHit>> caloCollectionMap;
  std::map<std::string, std::vector<ldmx::SimTrackerHit>> trackerCollectionMap;
  std::vector<ldmx::SimCalorimeterHit> simHitsCalo;
  std::vector<ldmx::SimTrackerHit> simHitsTracker;
  std::map<int, ldmx::SimCalorimeterHit> hitMap;

  for (int iEv = 0; iEv < nEvsOverlay; iEv++) {
    if (verbosity_ > 2) {
      ldmx_log(debug) << "in loop: overlaying event " << iEv + 1 << " out of "
                      << nEvsOverlay;
    }

    // an overlay event wide time offset to be applied to all its hits.
    // TODO -- figure out if we should also randomly shift the time of the sim
    // event (likely only needed if time bias gets picked up by BDT or ML by way
    // of pulse behaviour)
    float timeOffset = rndmTime_->Gaus(timeMean_, timeSigma_);
    int bunchOffset = (int)rndmTime_->Uniform(
        -(nBunchesToSample_ + 1),
        nBunchesToSample_ + 1);  // +1 to get inclusive interval
    float bunchTimeOffset = bunchSpacing_ * bunchOffset;
    timeOffset += bunchTimeOffset;

    if (verbosity_ > 2) {
      ldmx_log(debug) << "hit time offset in event " << iEv + 1 << " is  "
                      << timeOffset << " ns";
      ldmx_log(debug) << "bunch position offset in event " << iEv + 1 << " is  "
                      << bunchOffset
                      << ", leading to an additional time offset of "
                      << bunchTimeOffset << " ns";
    }

    /* ----------- first do the SimCalorimeterHits ----------- */

    // get the calo hits collections that we want to overlay, by looping over
    // the list of collections passed to the producer : caloCollections_
    for (uint iColl = 0; iColl < caloCollections_.size(); iColl++) {
      // for now, Ecal and only Ecal uses contribs instead of multiple
      // SimHitsCalo per channel, meaning, it requires special treatment
      bool needsContribsAdded = false;
      if (strstr(caloCollections_[iColl].c_str(), "Ecal"))
        needsContribsAdded = true;

      std::vector<ldmx::SimCalorimeterHit> overlayHits =
          overlayEvent_.getCollection<ldmx::SimCalorimeterHit>(
              caloCollections_[iColl], overlayPassName_);
      std::string outCollName = caloCollections_[iColl] + "Overlay";

      // if we alredy added at least one overlay event, this collection already
      // exists in the output collection map otherwise, start out by just
      // copying the sim hits, unaltered.
      if (caloCollectionMap.find(outCollName) == caloCollectionMap.end()) {
        simHitsCalo = event.getCollection<ldmx::SimCalorimeterHit>(
            caloCollections_[iColl], simPassName_);
        // but don't copy ecal hits immediately: for them, wait until overlay
        // contribs have been added. then add everything through the hitmap
        if (!needsContribsAdded) {
          caloCollectionMap[outCollName] = simHitsCalo;
        }

        if (verbosity_ > 2) {
          ldmx_log(debug) << "in loop: start of collection "
                          << caloCollections_[iColl];
          ldmx_log(debug) << "in loop: printing current sim event: ";
        }
        ldmx_log(debug) << "in loop: size of sim hits vector "
                        << caloCollections_[iColl] << " is "
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
        }    // if we need to enter this loop at all
      }  // if output collection doesn't already exist (i.e., we're in the first
         // overlay event)

      /* ----- now do calo hits overlay ----- */

      if (verbosity_ > 2) {
        ldmx_log(debug) << "in loop: printing overlay event: ";
      }
      ldmx_log(debug) << "in loop: size of overlay hits vector is "
                      << overlayHits.size();

      for (ldmx::SimCalorimeterHit &overlayHit : overlayHits) {
        if (verbosity_ > 2) overlayHit.Print();

        const float overlayTime = overlayHit.getTime() + timeOffset;
        overlayHit.setTime(overlayTime);

        if (needsContribsAdded) {  // special treatment for (for now only) ecal
          int overlayHitID = overlayHit.getID();
          if (hitMap.find(overlayHitID) ==
              hitMap.end()) {  // there wasn't already a simhit in this id
            hitMap[overlayHitID] = ldmx::SimCalorimeterHit();
            hitMap[overlayHitID].setID(overlayHitID);
            std::vector<float> hitPos = overlayHit.getPosition();
            hitMap[overlayHitID].setPosition(hitPos[0], hitPos[1], hitPos[2]);
          }
          // add the overlay hit (as a) contrib
          // incidentID = -1000, trackID = -1000, pdgCode = 0  <-- these are set
          // in the header for now but could be parameters
          hitMap[overlayHitID].addContrib(overlayIncidentID_, overlayTrackID_,
                                          overlayPdgCode_, overlayHit.getEdep(),
                                          overlayTime);
        }  // if add overlay as contribs
        else {
          caloCollectionMap[outCollName].push_back(overlayHit);
          if (verbosity_ > 2)
            ldmx_log(debug) << "Adding non-Ecal overlay hit to outhit vector "
                            << outCollName;
        }
      }  // over overlay calo simhit collection

      if (!needsContribsAdded)
        ldmx_log(debug) << "Nhits in overlay collection " << outCollName << ": "
                        << caloCollectionMap[outCollName].size();

    }  // over caloCollections

    /* ----------- now do the same with SimTrackerHits! ----------- */

    // get the SimTrackerHit collections that we want to overlay, by looping
    // over the list of collections passed to the producer : trackerCollections_
    for (uint iColl = 0; iColl < trackerCollections_.size(); iColl++) {
      std::vector<ldmx::SimTrackerHit> overlayTrackerHits =
          overlayEvent_.getCollection<ldmx::SimTrackerHit>(
              trackerCollections_[iColl], overlayPassName_);
      std::string outCollName = trackerCollections_[iColl] + "Overlay";

      // if we alredy added at least one overlay event, this collection already
      // exists in the output collection map otherwise, start out by just
      // copying the sim hits, unaltered.
      if (trackerCollectionMap.find(outCollName) ==
          trackerCollectionMap.end()) {
        simHitsTracker = event.getCollection<ldmx::SimTrackerHit>(
            trackerCollections_[iColl], simPassName_);
        trackerCollectionMap[outCollName] = simHitsTracker;

        // the rest is printouts for debugging
        ldmx_log(debug) << "in loop: size of sim hits vector "
                        << trackerCollections_[iColl] << " is "
                        << simHitsTracker.size();

        if (verbosity_ > 2) {
          ldmx_log(debug) << "in loop: start of collection "
                          << trackerCollections_[iColl];
          ldmx_log(debug) << "in loop: printing current sim event: ";

          for (const ldmx::SimTrackerHit &simHit : simHitsTracker) {
            if (verbosity_ > 2) simHit.Print();
          }  // over tracker simhit collection
        }    // if high verbosity
      }  // if output collection doesn't already exist (i.e., we're in the first
         // overlay event)

      /* ----- now do tracker hits overlay ---- */

      if (verbosity_ > 2) {
        ldmx_log(debug) << "in loop: printing overlay event: ";
      }
      ldmx_log(debug) << "in loop: size of overlay hits vector is "
                      << overlayTrackerHits.size();

      for (ldmx::SimTrackerHit &overlayHit : overlayTrackerHits) {
        const float overlayTime = overlayHit.getTime() + timeOffset;
        overlayHit.setTime(overlayTime);
        trackerCollectionMap[outCollName].push_back(overlayHit);

        if (verbosity_ > 2) {
          overlayHit.Print();
          ldmx_log(debug) << "Adding tracker overlay hit to outhit vector "
                          << outCollName;
        }
      }  // over overlay tracker simhit collection

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
  //	overlayFile_ = std::make_unique<framework::EventFile>( overlayFileName_,
  //true );
  overlayFile_ = std::make_unique<framework::EventFile>(overlayFileName_);
  overlayFile_->setupEvent(&overlayEvent_);

  // we update the iterator at the end of each event. so do this once here to
  // grab the first event in the processor
  // TODO this could also be done N random times to get a randomness in which
  // events get matched to what sim event. noticed that shifting by a fair chunk
  // helps remove some weak but suspicious correlations between sim and overlay
  // particle positions. leave it hardwired until we can reset the overlay event
  // counter in nextEvent() (implemented with the EventFile definition above)
  int nEventsShift_ = 23;
  for (int iShift = 0; iShift < nEventsShift_; iShift++) {
    if (!overlayFile_->nextEvent()) {
      std::cerr << "Couldn't read next event!";
      return;
    }
  }

  if (verbosity_ > 2) {
    ldmx_log(debug) << "onProcessStart () successful. Used input file: "
                    << overlayFile_->getFileName();
    ldmx_log(debug) << "onProcessStart () successful. Got event info: ";
    overlayFile_->getEvent()->Print(verbosity_);
  }

  return;
}

}  // namespace recon

DECLARE_PRODUCER_NS(recon, OverlayProducer)
