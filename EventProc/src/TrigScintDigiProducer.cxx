#include "EventProc/TrigScintDigiProducer.h"

#include <iostream>
#include <exception>

namespace ldmx {

    TrigScintDigiProducer::TrigScintDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    TrigScintDigiProducer::~TrigScintDigiProducer() {
    }

    void TrigScintDigiProducer::configure(Parameters& parameters) {

        // Configure this instance of the producer
        stripsPerArray_   = parameters.getParameter< int >("number_of_strips");
        numberOfArrays_   = parameters.getParameter< int >("number_of_arrays");
        meanNoise_        = parameters.getParameter< double >("mean_noise");
        mevPerMip_        = parameters.getParameter< double >("mev_per_mip");
        pePerMip_         = parameters.getParameter< double >("pe_per_mip");
        inputCollection_  = parameters.getParameter< std::string >("input_collection");
        inputPassName_    = parameters.getParameter< std::string >("input_pass_name" );
        outputCollection_ = parameters.getParameter< std::string >("output_collection");
        verbose_          = parameters.getParameter< bool >("verbose");

        random_ = std::make_unique<TRandom3>(parameters.getParameter< int >("randomSeed"));
        
        noiseGenerator_ = std::make_unique<NoiseGenerator>(meanNoise_, false); 
        noiseGenerator_->setNoiseThreshold(1); 
    }

    TrigScintID TrigScintDigiProducer::generateRandomID(int module) {

	TrigScintID tempID(module,random_->Integer(stripsPerArray_));
        if ( module >= TrigScintSection::NUM_SECTIONS ) {
            // Throw an exception
            std::cout << "WARNING [TrigScintDigiProducer::generateRandomID]: TrigScintSection is not known" << std::endl;
        }

        return tempID;
    }

    void TrigScintDigiProducer::produce(Event& event) {

        std::map<TrigScintID, int>   cellPEs;
        std::map<TrigScintID, int>   cellMinPEs;
        std::map<TrigScintID, float> Xpos, Ypos, Zpos, Edep, Time, beamFrac;
        std::set<TrigScintID> noiseHitIDs;

        auto numRecHits{0};

        // looper over sim hits and aggregate energy depositions for each detID
        const auto simHits{event.getCollection< SimCalorimeterHit >(inputCollection_,inputPassName_)};
        auto particleMap{event.getMap< int, SimParticle >("SimParticles")};

        int module{-1}; 
        for (const auto& simHit : simHits) {          

	    TrigScintID id(simHit.getID());

            // Just set the module ID to use for noise hits here.  Given that
            // we are currently processing a single module at a time, setting
            // it within the loop shouldn't matter.
            module = id.module(); 
            std::vector<float> position = simHit.getPosition();

            if (verbose_) {
                std::cout << id << std::endl;
	    }        

	    unsigned int detIDRaw = id.raw();
	    
            // check if hits is from beam electron and, if so, add to beamFrac
            for( int i = 0 ; i < simHit.getNumberOfContribs() ; i++){
	       
                auto contrib = simHit.getContrib(i);
                if( verbose_ ){
                    std::cout << "contrib " << i << " trackID: " << contrib.trackID << " pdgID: " << contrib.pdgCode << " edep: " << contrib.edep << std::endl;
                    std::cout << "\t particle id: " << particleMap[contrib.trackID].getPdgID() << " particle status: " << particleMap[contrib.trackID].getGenStatus() << std::endl;
                }
                if( particleMap[contrib.trackID].getPdgID() == 11 && particleMap[contrib.trackID].getGenStatus() == 1 ){
                    if( beamFrac.find(id) == beamFrac.end() )
                        beamFrac[id]=contrib.edep;
                    else
                        beamFrac[id]+=contrib.edep;                          
                }
            }

            // for now, we take am energy weighted average of the hit in each stip to simulate the hit position. 
            // AJW: these should be dropped, they are likely to lead to a problem since we can't measure them anyway
            // except roughly y and z, which is encoded in the ids.
            if (Edep.find(id) == Edep.end()) {

                // first hit, initialize
                Edep[id] = simHit.getEdep();
                Time[id] = simHit.getTime() * simHit.getEdep();
                Xpos[id] = position[0]* simHit.getEdep();
                Ypos[id] = position[1]* simHit.getEdep();
                Zpos[id] = position[2]* simHit.getEdep();
                numRecHits++;

            } else {

                // not first hit, aggregate, and store the largest radius hit
                Xpos[id] += position[0]* simHit.getEdep();
                Ypos[id] += position[1]* simHit.getEdep();
                Zpos[id] += position[2]* simHit.getEdep();
                Edep[id] += simHit.getEdep();
                // AJW: need to figure out a better way to model this...
                Time[id] += simHit.getTime() * simHit.getEdep();
            }

        }

        // Create the container to hold the digitized trigger scintillator hits.
        std::vector< TrigScintHit > trigScintHits; 

        // loop over detIDs and simulate number of PEs
        int ihit = 0;        
        for (std::map<TrigScintID, float>::iterator it = Edep.begin(); it != Edep.end(); ++it) {
	    TrigScintID id(it->first);

            double depEnergy    = Edep[id];
            Time[id]      = Time[id] / Edep[id];
            Xpos[id]      = Xpos[id] / Edep[id];
            Ypos[id]      = Ypos[id] / Edep[id];
            Zpos[id]      = Zpos[id] / Edep[id];
            double meanPE       = depEnergy / mevPerMip_ * pePerMip_;
            cellPEs[id]   = random_->Poisson(meanPE + meanNoise_);



            // If a cell has a PE count above threshold, persit the hit.
            if( cellPEs[id] >= 1 ){ 

                TrigScintHit hit; 
                hit.setID(id.raw());
                hit.setPE(cellPEs[id]);
                hit.setMinPE(cellMinPEs[id]);
                hit.setAmplitude(cellPEs[id]);
                hit.setEnergy(depEnergy);
                hit.setTime(Time[id]);
                hit.setXPos(Xpos[id]); 
                hit.setYPos(Ypos[id]); 
                hit.setZPos(Zpos[id]);
                hit.setModuleID(module);
                hit.setBarID(id.bar() ); //getFieldValue("bar"));
                hit.setNoise(false);
                hit.setBeamEfrac(beamFrac[id]/depEnergy);

                trigScintHits.push_back(hit); 
            }

            if (verbose_) {
		
                std::cout << id << std::endl; 
                std::cout << "Edep: " << Edep[id] << std::endl;
                std::cout << "numPEs: " << cellPEs[id] << std::endl;
                std::cout << "time: " << Time[id] << std::endl;std::cout << "z: " << Zpos[id] << std::endl;
                std::cout << "\t X: " << Xpos[id] <<  "\t Y: " << Ypos[id] <<  "\t Z: " << Zpos[id] << std::endl;
            }        // end verbose            
        }

        // ------------------------------- Noise simulation -------------------------------
        int numEmptyCells = stripsPerArray_ - numRecHits; // only simulating for single array until all arrays are merged into one collection
        std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits( numEmptyCells );

        TrigScintID tempID;

        for (auto& noiseHitPE : noiseHits_PE) {

            TrigScintHit hit;
            // generate random ID from remaining cells
            do {
                tempID = generateRandomID(module);
            } while( Edep.find(tempID) != Edep.end() || 
                    noiseHitIDs.find(tempID) != noiseHitIDs.end() );

            TrigScintID noiseID=tempID;

	        noiseHitIDs.insert(noiseID);

            hit.setID(noiseID.raw());
            hit.setPE(noiseHitPE);
            hit.setMinPE(noiseHitPE);
            hit.setAmplitude(noiseHitPE);
            hit.setEnergy(0.);
            hit.setTime(0.);
            hit.setXPos(0.);
            hit.setYPos(0.);
            hit.setZPos(0.);
            hit.setModuleID(module);
            hit.setBarID(noiseID.bar());
            hit.setNoise(true);
            hit.setBeamEfrac(0.);

            trigScintHits.push_back(hit); 
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        event.add(outputCollection_, trigScintHits);
    }
}

DECLARE_PRODUCER_NS(ldmx, TrigScintDigiProducer);

