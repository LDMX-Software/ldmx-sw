#include "TClonesArray.h"

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
        
        detID_ = std::make_unique<TrigScintID>();
        
        noiseGenerator_ = std::make_unique<NoiseGenerator>(meanNoise_, false); 
        noiseGenerator_->setNoiseThreshold(1); 
    }

    unsigned int TrigScintDigiProducer::generateRandomID(int module) {

        TrigScintID tempID;
        if ( module < TrigScintSection::NUM_SECTIONS ) {
            tempID.setFieldValue("module", module);
            tempID.setFieldValue("bar", random_->Integer(stripsPerArray_));
        } else { 
            // Throw an exception
            std::cout << "WARNING [TrigScintDigiProducer::generateRandomID]: TrigScintSection is not known" << std::endl;
        }

        return tempID.pack();
    }

    void TrigScintDigiProducer::produce(Event& event) {

        std::map<unsigned int, int>   cellPEs;
        std::map<unsigned int, int>   cellMinPEs;
        std::map<unsigned int, float> Xpos, Ypos, Zpos, Edep, Time;
        std::unordered_set<unsigned int> noiseHitIDs;

        auto numRecHits{0};

        // looper over sim hits and aggregate energy depositions for each detID
        const auto simHits{event.getCollection< SimCalorimeterHit >(inputCollection_,inputPassName_)};

        int module{-1}; 
        for (const auto& simHit : simHits) {          

            int detIDRaw{simHit.getID()};
            detID_->setRawValue(detIDRaw);
            detID_->unpack();

            // Just set the module ID to use for noise hits here.  Given that
            // we are currently processing a single module at a time, setting
            // it within the loop shouldn't matter.
            module = detID_->getFieldValue("module"); 
            std::vector<float> position = simHit.getPosition();

            if (verbose_) {
                std::cout << "Module: " << module 
                          << " Bar: " << detID_->getFieldValue("bar") 
                          << std::endl; 
            }        

            // for now, we take am energy weighted average of the hit in each stip to simulate the hit position. 
            // will use strip TOF and light yield between strips to estimate position.            
            if (Edep.find(detIDRaw) == Edep.end()) {

                // first hit, initialize
                Edep[detIDRaw] = simHit.getEdep();
                Time[detIDRaw] = simHit.getTime() * simHit.getEdep();
                Xpos[detIDRaw] = position[0]* simHit.getEdep();
                Ypos[detIDRaw] = position[1]* simHit.getEdep();
                Zpos[detIDRaw] = position[2]* simHit.getEdep();
                numRecHits++;

            } else {

                // not first hit, aggregate, and store the largest radius hit
                Xpos[detIDRaw] += position[0]* simHit.getEdep();
                Ypos[detIDRaw] += position[1]* simHit.getEdep();
                Zpos[detIDRaw] += position[2]* simHit.getEdep();
                Edep[detIDRaw] += simHit.getEdep();
                // AJW: need to figure out a better way to model this...
                Time[detIDRaw] += simHit.getTime() * simHit.getEdep();
            }

        }

        // Create the container to hold the digitized trigger scintillator hits.
        std::vector< TrigScintHit > trigScintHits; 

        // loop over detIDs and simulate number of PEs
        int ihit = 0;        
        for (std::map<unsigned int, float>::iterator it = Edep.begin(); it != Edep.end(); ++it) {
            int detIDRaw = it->first;
            double depEnergy    = Edep[detIDRaw];
            Time[detIDRaw]      = Time[detIDRaw] / Edep[detIDRaw];
            Xpos[detIDRaw]      = Xpos[detIDRaw] / Edep[detIDRaw];
            Ypos[detIDRaw]      = Ypos[detIDRaw] / Edep[detIDRaw];
            Zpos[detIDRaw]      = Zpos[detIDRaw] / Edep[detIDRaw];
            double meanPE       = depEnergy / mevPerMip_ * pePerMip_;
            cellPEs[detIDRaw]   = random_->Poisson(meanPE + meanNoise_);



            // If a cell has a PE count above threshold, persit the hit.
            if( cellPEs[detIDRaw] >= 1 ){ 
                
                detID_->setRawValue(detIDRaw);
                detID_->unpack();
                auto bar{detID_->getFieldValue("bar")};

                TrigScintHit hit; 
                hit.setID(detIDRaw);
                hit.setPE(cellPEs[detIDRaw]);
                hit.setMinPE(cellMinPEs[detIDRaw]);
                hit.setAmplitude(cellPEs[detIDRaw]);
                hit.setEnergy(depEnergy);
                hit.setTime(Time[detIDRaw]);
                hit.setXpos(Xpos[detIDRaw]); 
                hit.setYpos(Ypos[detIDRaw]); 
                hit.setZpos(Zpos[detIDRaw]);
                hit.setModuleID(module);
                hit.setBarID(detID_->getBarID() ); //getFieldValue("bar"));
                hit.setNoise(false);

                trigScintHits.push_back(hit); 

            }

            if (verbose_) {
                detID_->setRawValue(detIDRaw);
                detID_->unpack();
                auto bar{detID_->getFieldValue("bar")};

                std::cout << "Module: " << module << " Bar: " << bar << std::endl; 
                std::cout << "Edep: " << Edep[detIDRaw] << std::endl;
                std::cout << "numPEs: " << cellPEs[detIDRaw] << std::endl;
                std::cout << "time: " << Time[detIDRaw] << std::endl;std::cout << "z: " << Zpos[detIDRaw] << std::endl;
                std::cout << "\t X: " << Xpos[detIDRaw] <<  "\t Y: " << Ypos[detIDRaw] <<  "\t Z: " << Zpos[detIDRaw] << std::endl;
            }        // end verbose            
        }

        // ------------------------------- Noise simulation -------------------------------
        int numEmptyCells = stripsPerArray_ - numRecHits; // only simulating for single array until all arrays are merged into one collection
        std::vector<double> noiseHits_PE = noiseGenerator_->generateNoiseHits( numEmptyCells );

        int tempID;

        for (auto& noiseHitPE : noiseHits_PE) {

            TrigScintHit hit;
            // generate random ID from remaining cells
            do {
                tempID = generateRandomID(module);
            } while( Edep.find(tempID) != Edep.end() || 
                    noiseHitIDs.find(tempID) != noiseHitIDs.end() );

	          TrigScintID noiseID;
	          noiseID.setRawValue(tempID);
            noiseID.unpack();

            hit.setID(tempID);
            hit.setPE(noiseHitPE);
            hit.setMinPE(noiseHitPE);
            hit.setAmplitude(noiseHitPE);
            hit.setEnergy(0.);
            hit.setTime(0.);
            hit.setXpos(0.);
            hit.setYpos(0.);
            hit.setZpos(0.);
	          hit.setModuleID(module);
            hit.setBarID(noiseID.getFieldValue("bar"));
            hit.setNoise(true);

            trigScintHits.push_back(hit); 
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        event.add(outputCollection_, trigScintHits);
    }
}

DECLARE_PRODUCER_NS(ldmx, TrigScintDigiProducer);

