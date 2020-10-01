/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 * @author Cristina Suarez, Fermi National Accelerator Laboratory
 */

#include "Hcal/HcalDigiProducer.h"
#include "Framework/RandomNumberSeedService.h"

namespace ldmx {

    HcalDigiProducer::HcalDigiProducer(const std::string& name, Process& process) :
        Producer(name, process) {

        //noise generator by default uses a Gausian model for noise
        //  i.e. It assumes the noise is distributed around a mean (setPedestal)
        //  with a certain RMS (setNoise) and then calculates
        //  how many hits should be generated for a given number of empty
        //  channels and a minimum readout value (setNoiseThreshold)
        noiseGenerator_ = std::make_unique<NoiseGenerator>();
    }

    HcalDigiProducer::~HcalDigiProducer() { }

    void HcalDigiProducer::configure(Parameters& ps) {

        //settings of readout chip
        //  used  in actual digitization
        auto hgcrocParams = ps.getParameter<Parameters>("hgcroc");
        hgcroc_ = std::make_unique<HgcrocEmulator>(hgcrocParams);
        gain_             = hgcrocParams.getParameter<double>("gain");
        pedestal_         = hgcrocParams.getParameter<double>("pedestal");
        clockCycle_       = hgcrocParams.getParameter<double>("clockCycle");
        nADCs_            = hgcrocParams.getParameter<int>("nADCs");
        iSOI_             = hgcrocParams.getParameter<int>("iSOI");

        // physical constants
        //  used to calculate unit conversions
        MeV_ = ps.getParameter<double>("MeV");

        //Time -> clock counts conversion
        //  time [ns] * ( 2^10 / max time in ns ) = clock counts
        ns_ = 1024./clockCycle_;

        // Configure generator that will produce noise hits in empty channels
        readoutThreshold_ = hgcrocParams.getParameter<double>("readoutThreshold");
    }

    void HcalDigiProducer::produce(Event& event) {

        if(!hgcroc_->hasSeed()) {
            const auto& rseed = getCondition<RandomNumberSeedService>(RandomNumberSeedService::CONDITIONS_OBJECT_NAME);
            hgcroc_->seedGenerator(rseed.getSeed("HcalDigiProducer::HgcrocEmulator"));
        }

        //Empty collection to be filled
        HgcrocDigiCollection hcalDigis;
        hcalDigis.setNumSamplesPerDigi( nADCs_ ); 
        hcalDigis.setSampleOfInterestIndex( iSOI_ );

        std::set<unsigned int> filledDetIDs; //detector IDs that already have a hit in them

        /******************************************************************************************
         * HGCROC Emulation on Simulated Hits
         *****************************************************************************************/
        //std::cout << "Sim Hits" << std::endl;
        //get simulated hcal hits from Geant4
        //  the class HcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
        //  this class ensures that only one SimCalorimeterHit is generated per cell, but
        //  multiple "contributions" are still handled within SimCalorimeterHit 
        auto hcalSimHits{event.getCollection<SimCalorimeterHit>(EventConstants::HCAL_SIM_HITS)};

        // debug printout
        std::cout << "Energy to Voltage Conversion: " << MeV_ << " mV/MeV" << std::endl;

        for (auto const& simHit : hcalSimHits ) {

            std::vector<double> voltages, times;
	    double sumEdep = 0; // should be the same as simHit.getEdep() 
            int sumPE = 0;
            int maxPE = 0;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {
                /* debug printout
                std::cout << simHit.getContrib(iContrib).edep << " MeV" << std::endl;
                 */
                voltages.push_back( simHit.getContrib( iContrib ).edep*MeV_ );
                times.push_back( simHit.getContrib( iContrib ).time );
		// debug info
		sumEdep += simHit.getContrib( iContrib ).edep;
                int PE = simHit.getContrib( iContrib ).edep*(1/4.66)*68;
                sumPE += PE;
                if(PE > maxPE)
                  maxPE = PE;
            }

            unsigned int hitID = simHit.getID();
            filledDetIDs.insert( hitID );
	    HcalID detID(hitID);
            int layer = detID.layer();
            int subsection = detID.section();
            int strip = detID.strip();

            //container emulator uses to write out samples and
            //transfer samples into the digi collection
            std::vector<HgcrocDigiCollection::Sample> digiToAdd;
            if ( hgcroc_->digitize( hitID , voltages , times , digiToAdd ) ) {
	        digiToAdd.edep_ = sumEdep;
	        digiToAdd.pe_ = sumPE;
	        digiToAdd.maxpe_ = maxPE;
	        digiToAdd.strip_ = strip;
	        digiToAdd.subsection_ = subsection;
	        digiToAdd.layer_ = layer;
                hcalDigis.addDigi( hitID , digiToAdd );
            }
        }

        event.add("HcalDigis", hcalDigis );

        return;
    } //produce

}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);
