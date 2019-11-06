/**
 * @file EcalRecProducer.cxx
 * @brief Class that performs basic ECal reconstruction
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Ecal/EcalRecProducer.h"

namespace ldmx {

    const std::vector<double> EcalRecProducer::DEFAULT_LAYER_WEIGHTS 
        = {1.641, 3.526, 5.184, 6.841,
        8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
        8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
        16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45}; 

    EcalRecProducer::EcalRecProducer(const std::string& name, Process& process) :
        Producer(name, process) {
    }

    EcalRecProducer::~EcalRecProducer() {
        if ( ecalRecHits_ ) delete ecalRecHits_;
    }

    void EcalRecProducer::configure(const ParameterSet& ps) {

        digiCollName_ = ps.getString( "digiCollName" , "EcalDigis" );
        digiPassName_ = ps.getString( "digiPassName" , "" );

        layerWeights_ = ps.getVDouble( "layerWeights" , DEFAULT_LAYER_WEIGHTS );
        secondOrderEnergyCorrection_ = ps.getDouble( "secondOrderEnergyCorrection" , DEFAULT_SECOND_ORDER_ENERGY_CORRECTION );

        ecalRecHits_ = new TClonesArray( EventConstants::ECAL_HIT.c_str(), 10000 );
    }

    void EcalRecProducer::produce(Event& event) {

        TClonesArray* ecalDigis = (TClonesArray*) event.getCollection( digiCollName_ , digiPassName_ );
        int totalNL1Samples = ecalDigis->GetEntriesFast();

        //loop through all digis and sort based off of detector ID
        std::map< int , std::vector< EcalDigi *> > detID_EcalDigi;
        for (int iDigi = 0; iDigi < totalNL1Samples; iDigi++) {
            EcalDigi *currDigi = (EcalDigi *)( ecalDigis->At( iDigi ) );
            detID_EcalDigi[ currDigi->getID() ].push_back( currDigi );
        }

        //loop through map of hex coordinates
        std::map< int , std::vector< EcalDigi *> >::iterator it_detID_EcalDigi;
        int iHit = 0;
        for ( it_detID_EcalDigi = detID_EcalDigi.begin() ; it_detID_EcalDigi == detID_EcalDigi.end(); ++it_detID_EcalDigi ) {
            
            int rawID = it_detID_EcalDigi->first;
            detID_.setRawValue( rawID );
            detID_.unpack();

            int cellID   = detID_.getFieldValue( 3 );
            int moduleID = detID_.getFieldValue( 2 );
            int layer    = detID_.getLayerID();

            //cell, module, layer IDs to real space position
            XYCoords cellCenter = ecalHexReadout_.getCellCenterAbsolute( ecalHexReadout_.combineID( cellID , moduleID ) );
            //z position requires some encoding of the distance from the target

            //get energy and time estimate from digi information
            double siEnergy, hitTime;
            
            //TODO: Energy estimate from N samples can (and should be) refined
            //TOA is the time of arrival with respect to the 25ns clock window
            double timeRelClock25 = it_detID_EcalDigi->second.at(0)->getTOA()*(25./pow(2.,10)); //ns

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

            //TOT - number of clock ticks that pulse was over threshold
            //  this is related to the amplitude of the pulse through some convoluted relation using the pulse shape
            //  the amplitude of the pulse is related to the energy deposited
            //  for now, we fit TOT vs EDep (from SimHit) and use that fit to do this transformation
            
            //incorporate layer weights
            double recHitEnergy = 
                ( (siEnergy / MIP_SI_RESPONSE)*layerWeights_.at(layer)+siEnergy )*secondOrderEnergyCorrection_;

            //copy over information to rec hit structure in new collection
            EcalHit *recHit = (EcalHit *)( ecalRecHits_->ConstructedAt( iHit ) );
            recHit->setID( rawID );
            recHit->setAmplitude( siEnergy );
            recHit->setEnergy( recHitEnergy );
            recHit->setTime( hitTime );

            iHit++;
        }

        //add collection to event bus
        event.add( "EcalRecHits", ecalRecHits_ );
    }
}

DECLARE_PRODUCER_NS(ldmx, EcalRecProducer);
