/**
 * @file EcalRecProducer.cxx
 * @brief Class that performs basic ECal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalRecProducer.h"
#include "Ecal/EcalReconConditions.h"
#include "DetDescr/EcalHexReadout.h"

namespace ldmx {

    EcalRecProducer::EcalRecProducer(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    EcalRecProducer::~EcalRecProducer() { }

    void EcalRecProducer::configure(Parameters& ps) {

        //collection names
        digiCollName_   = ps.getParameter<std::string>("digiCollName");
        digiPassName_   = ps.getParameter<std::string>("digiPassName");
        simHitCollName_ = ps.getParameter<std::string>("simHitCollName");
        simHitPassName_ = ps.getParameter<std::string>("simHitPassName");
        recHitCollName_ = ps.getParameter<std::string>("recHitCollName");

        layerWeights_ = ps.getParameter<std::vector<double>>( "layerWeights" );
        secondOrderEnergyCorrection_ = ps.getParameter<double>( "secondOrderEnergyCorrection" );

        mip_si_energy_  = ps.getParameter<double>( "mip_si_energy"  );
        clock_cycle_    = ps.getParameter<double>( "clock_cycle"    );
        charge_per_mip_ = ps.getParameter<double>( "charge_per_mip" );

    }

    void EcalRecProducer::produce(Event& event) {
        // Get the Ecal Geometry
        const EcalHexReadout& hexReadout = getCondition<EcalHexReadout>(EcalHexReadout::CONDITIONS_OBJECT_NAME);

        // Get the reconstruction parameters
        EcalReconConditions the_conditions(getCondition<DoubleTableCondition>(EcalReconConditions::CONDITIONS_NAME));

        std::vector<EcalHit> ecalRecHits;
        auto ecalDigis = event.getObject<HgcrocDigiCollection>( digiCollName_ , digiPassName_ );
        int numDigiHits = ecalDigis.getNumDigis();
        //loop through digis
        for ( unsigned int iDigi = 0; iDigi < numDigiHits; iDigi++ ) {
            
            auto digi = ecalDigis.getDigi( iDigi );

            //ID from first digi sample
            //  assuming rest of samples have same ID
            EcalID id(digi.id());
            
            //ID to real space position
            double x,y,z;
            hexReadout.getCellAbsolutePosition( id , x , y , z );

            //TOA is the time of arrival with respect to the 25ns clock window
            //  TODO what to do if hit NOT in first clock cycle?
            double timeRelClock25 = digi.begin()->toa()*(clock_cycle_/1024); //ns
            double hitTime = timeRelClock25;

            //get the estimated charge deposited from digi samples
            double charge(0.);

            /* debug printout
            std::cout << "Recon { "
                << "ID: " << id << ", "
                << "TOA: " << hitTime << "ns } ";
                */
            if ( digi.isTOT() ) {
                //TOT - number of clock ticks that pulse was over threshold
                //  this is related to the amplitude of the pulse approximately through a linear drain rate
                //  the amplitude of the pulse is related to the energy deposited

                //convert the time over threshold into a total energy deposited in the silicon
                //  (time over threshold [ns] - pedestal) * gain
                charge = (digi.tot() - the_conditions.totPedestal(id))*the_conditions.totGain(id);

                /* debug printout
                std::cout << "TOT Mode -> " << digi.tot() << "TDC -> " << charge << " fC";
                 */
            } else {
                //ADC mode of readout
                //ADC - voltage measurement at a specific time of the pulse
                // Pulse Shape:
                //  p[0]/(1.0+exp(p[1](t-p[2]+p[3]-p[4])))/(1.0+exp(p[5]*(t-p[6]+p[3]-p[4])))
                //  p[0] = amplitude to be fit (TBD)
                //  p[1] = -0.345 shape parameter - rate of up slope
                //  p[2] = 70.6547 shape parameter - time of up slope relative to shape fit
                //  p[3] = 77.732 shape parameter - time of peak relative to shape fit
                //  p[4] = peak time to be fit (TBD)
                //  p[5] = 0.140068 shape parameter - rate of down slope
                //  p[6] = 87.7649 shape paramter - time of down slope relative to shape fit
                //These measurements can be used to fit the pulse shape if TOT is not available
                
                TH1F measurements( "measurements" , "measurements" ,
                        10.*clock_cycle_ , 0. , 10.*clock_cycle_ );

                double maxMeas{0.};
                int numWholeClocks{0};
                for ( auto it = digi.begin(); it < digi.end(); it++) {
                    double amplitude_fC = (it->adc_t() - the_conditions.adcPedestal(id))*the_conditions.adcGain(id);
                    double time          = numWholeClocks*clock_cycle_; //+ offestWithinClock; //ns
                    measurements.Fill( time , amplitude_fC );
                    if ( amplitude_fC > maxMeas ) maxMeas = amplitude_fC;
                }

                if ( false ) {
                    //fit the voltage measurements with the pulse function
                    //  would need to access the pulse function in HGCROC somehow
                    //voltageMeasurements.Fit( &pulseFunc_ , "QW" );
                    //get the silicon energy from the fitted voltage amplitude in mV
                    //siEnergy = (pulseFunc_.GetParameter( 0 ))*mV_;
                } else {
                    //just use the maximum measured voltage
                    charge = maxMeas;
                }

                /* debug printout
                std::cout << "ADC Mode -> " << charge << " fC";
                 */
            }

            double num_mips_equivalent = charge / charge_per_mip_;
            double energy_deposited_in_Si = num_mips_equivalent * mip_si_energy_;

            /* debug printout
            std::cout << " -> " << num_mips_equivalent
                << " equiv MIPs -> " << energy_deposited_in_Si << " MeV"
                << std::endl;
             */
            
            //incorporate layer weights
            double reconstructed_energy = 
                ( 
                  num_mips_equivalent*layerWeights_.at(id.layer()) //energy lost in non-sensitive layers
                  +energy_deposited_in_Si //energy deposited in Si itself
                )*secondOrderEnergyCorrection_;

            //copy over information to rec hit structure in new collection
            EcalHit recHit;
            recHit.setID( id.raw() );
            recHit.setXPos( x );
            recHit.setYPos( y );
            recHit.setZPos( z );
            recHit.setAmplitude( energy_deposited_in_Si );
            recHit.setEnergy( reconstructed_energy );
            recHit.setTime( hitTime );

            ecalRecHits.push_back( recHit );
        }

        if (event.exists( simHitCollName_, simHitPassName_ )) {
            //ecal sim hits exist ==> label which hits are real and which are pure noise
            auto ecalSimHits{event.getCollection<SimCalorimeterHit>( simHitCollName_, simHitPassName_ )};
            std::set<int> real_hits;
            for ( auto const& sim_hit : ecalSimHits ) real_hits.insert( sim_hit.getID() );
            for ( auto& hit : ecalRecHits ) hit.setNoise( real_hits.find(hit.getID()) == real_hits.end() );
        }

        //add collection to event bus
        event.add( recHitCollName_, ecalRecHits );
    }

}

DECLARE_PRODUCER_NS(ldmx, EcalRecProducer);
