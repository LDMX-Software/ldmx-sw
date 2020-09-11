/**
 * @file HcalDigiProducer.cxx
 * @brief Class that performs basic ECal digitization
 * @author Cameron Bravo, SLAC National Accelerator Laboratory
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Hcal/HcalDigiProducer.h"

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

        // geometry constants
        //  These are used in the noise generation so that we can randomly
        //  distribute the noise uniformly throughout the ECal channels.
        nHcalLayers_      = ps.getParameter<int>("nHcalLayers");
        nModulesPerLayer_ = ps.getParameter<int>("nModulesPerLayer");
        nCellsPerModule_  = ps.getParameter<int>("nCellsPerModule");
        nTotalChannels_   = nHcalLayers_*nModulesPerLayer_*nCellsPerModule_;

        // Configure generator that will produce noise hits in empty channels
        readoutThreshold_ = hgcrocParams.getParameter<double>("readoutThreshold");
        noiseGenerator_->setNoise(hgcrocParams.getParameter<double>("noiseRMS")); //rms noise in mV
        noiseGenerator_->setPedestal(gain_*pedestal_); //mean noise amplitude (if using Gaussian Model for the noise) in mV
        noiseGenerator_->setNoiseThreshold(readoutThreshold_); //threshold for readout in mV

        //The noise injector is used to place smearing on top
        //of energy depositions and hit times before doing
        //the digitization procedure.
        int seed = ps.getParameter<int>("randomSeed");
        noiseInjector_ = std::make_unique<TRandom3>(seed);
        noiseGenerator_->setSeed(seed);

    }

    void HcalDigiProducer::produce(Event& event) {

        //Empty collection to be filled
        HgcrocDigiCollection ecalDigis;
        ecalDigis.setNumSamplesPerDigi( nADCs_ ); 
        ecalDigis.setSampleOfInterestIndex( iSOI_ );

        std::set<int> filledDetIDs; //detector IDs that already have a hit in them

        /******************************************************************************************
         * HGCROC Emulation on Simulated Hits
         *****************************************************************************************/
        //std::cout << "Sim Hits" << std::endl;
        //get simulated ecal hits from Geant4
        //  the class HcalHitIO in the SimApplication module handles the translation from G4CalorimeterHits to SimCalorimeterHits
        //  this class ensures that only one SimCalorimeterHit is generated per cell, but
        //  multiple "contributions" are still handled within SimCalorimeterHit 
        auto ecalSimHits{event.getCollection<SimCalorimeterHit>(EventConstants::ECAL_SIM_HITS)};

        for (auto const& simHit : ecalSimHits ) {

            std::vector<double> voltages, times;
            for ( int iContrib = 0; iContrib < simHit.getNumberOfContribs(); iContrib++ ) {
                voltages.push_back( simHit.getContrib( iContrib ).edep*MeV_ );
                times.push_back( simHit.getContrib( iContrib ).time );
            }

            int hitID = simHit.getID();
            filledDetIDs.insert( hitID );
            //std::cout << hitID << " ";
            HgcrocDigiCollection::HgcrocDigi digiToAdd;
            if ( hgcroc_->digitize( hitID , voltages , times , digiToAdd ) ) {
                ecalDigis.addDigi( digiToAdd );
            }
        }

        /******************************************************************************************
         * Noise Simulation on Empty Channels
         *****************************************************************************************/
        if ( noise_ ) {
            //std::cout << "Noise Hits" << std::endl;
            //put noise into some empty channels
            int numEmptyChannels = nTotalChannels_ - ecalDigis.getNumDigis(); //minus number of channels with a hit
            //noise generator gives us a list of noise amplitudes [mV] that randomly populate the empty
            //channels and are above the readout threshold
            auto noiseHitAmplitudes{noiseGenerator_->generateNoiseHits(numEmptyChannels)};
            std::vector<double> voltages( 1 , 0.), times( 1 , 0. );
            for ( double noiseHit : noiseHitAmplitudes ) {
    
                //generate detector ID for noise hit
                //making sure that it is in an empty channel
                int noiseID;
                do {
                    int layerID  = noiseInjector_->Integer(nHcalLayers_);
                    int moduleID = noiseInjector_->Integer(nModulesPerLayer_);
                    int cellID   = noiseInjector_->Integer(nCellsPerModule_);
		    auto detID=HcalID(layerID, moduleID, cellID);
                    noiseID = detID.raw();
                } while ( filledDetIDs.find( noiseID ) != filledDetIDs.end() );
                filledDetIDs.insert( noiseID );
                //std::cout << noiseID << " -> " << noiseHit + readoutThreshold_ - gain_*pedestal_ << std::endl;

                //get a time for this noise hit
                times[0]    = noiseInjector_->Uniform( clockCycle_ );

                //noise generator gives the amplitude above the readout threshold
                //  we need to convert it to the amplitdue above the pedestal
                voltages[0] = noiseHit + readoutThreshold_ - gain_*pedestal_;
    
                HgcrocDigiCollection::HgcrocDigi digiToAdd;
                if ( hgcroc_->digitize( noiseID , voltages , times , digiToAdd ) ) {
                    ecalDigis.addDigi( digiToAdd );
                }
            } //loop over noise amplitudes
        } //if we should do the noise

        event.add("HcalDigis", ecalDigis );

        return;
    } //produce
}

DECLARE_PRODUCER_NS(ldmx, HcalDigiProducer);
