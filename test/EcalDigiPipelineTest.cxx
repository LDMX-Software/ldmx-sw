/**
 * @file EcalDigiPipelineTest.cxx
 * @brief Test to make sure input sim energies are close to output rec energies
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Framework/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/Event.h" //We need an event bus to pass through produce
#include "Framework/Process.h" //We need a dummy process to link the event bus to
#include "Ecal/EcalDigiProducer.h" //headers defining what we will be testing
#include "Ecal/EcalRecProducer.h" //headers defining what we will be testing

/**
 * Test for the Ecal Digi Pipeline
 *
 * Does not check for realism. Simply makes sure sim energies
 * end up being "close" to output rec energies.
 *  Close is defined right now to be a maximum of 2.5% relative difference.
 *
 * TODO still need to expand to multiple contribs in a single sim hit
 * TODO check ability to discard noise / low energy sim hits
 * TODO check layer weights are being calculated correctly somehow
 */
TEST_CASE( "Ecal Digi Pipeline test" , "[Ecal][functionality]" ) {

    using namespace ldmx;

    auto dummyProcess{Process::getDummy()};
    Event testEventBus( "testEcalDigiPipeline" );

    std::vector<SimCalorimeterHit> pretendSimHits;

    //fill pretend sim hits with a range of simulated energies
    double maxEnergy  = 25. ; //MeV
    double energyStep = 0.5 ; //MeV
    double currEnergy = 1.  ; //minEnergy MeV <-- needs to be above readout threshold (after internal EcalDigi's calculation)
    int index = 0;
    while ( currEnergy < maxEnergy ) {
        //TODO test several contribs
        //  right now, not a big deal because we just do an energy weight average anyways
        SimCalorimeterHit currHit;
	EcalID id(0,1,index++);
        currHit.setID( id.raw() );
        currHit.addContrib(
                -1 //incidentID
                , -1 // trackID
                , 0 // pdg ID
                , currEnergy // edep
                , 1. //time
                );
        pretendSimHits.push_back( currHit );

        currEnergy += energyStep;
    }

    testEventBus.add( "EcalSimHits" , pretendSimHits ); //needs to be correct collection name

    std::map<std::string,std::any> actualParameters;
    EcalDigiProducer digis( "testDigis" , dummyProcess );
    actualParameters.clear();
    actualParameters[ "randomSeed" ] = 1;
    actualParameters[ "gain" ] = 2000.;
    actualParameters[ "pedestal" ] = 1100.;
    actualParameters[ "noiseIntercept" ] = 700.;
    actualParameters[ "noiseSlope" ] = 25.;
    actualParameters[ "padCapacitance" ] = 0.1;
    actualParameters[ "nADCs" ] = 10; 
    actualParameters[ "iSOI"  ] = 0; 
    actualParameters[ "readoutThreshold" ] = 4.;
    actualParameters[ "makeConfigHists" ] = false;
    Parameters ecalDigi;
    ecalDigi.setParameters( actualParameters );
    REQUIRE_NOTHROW( digis.configure( ecalDigi ) );

    REQUIRE_NOTHROW( digis.produce( testEventBus ) );

    
    actualParameters.clear();
    actualParameters["gap"] = 1.5;
    actualParameters["moduleMinR"] = 85.0;
    actualParameters["ecalFrontZ"] = 240.5;
    actualParameters["nCellRHeight"] = 35.3;
    actualParameters["verbose"] = 0;
    std::vector<double> layerZPositions = {
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
    };
    actualParameters["layerZPositions"] = layerZPositions;
    Parameters hexReadout;
    hexReadout.setParameters(actualParameters);

    EcalRecProducer recon( "testRecon" , dummyProcess);
    actualParameters.clear();
    actualParameters[ "digiCollName" ] = std::string("EcalDigis");
    actualParameters[ "digiPassName" ] = std::string();
    actualParameters[ "secondOrderEnergyCorrection" ] = 4000./4010.;
    std::vector<double> layerWeights = {
        1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
        17.364, 8.990
    };
    actualParameters[ "layerWeights" ] = layerWeights;
    actualParameters[ "hexReadout"   ] = hexReadout;
    Parameters ecalRecon;
    ecalRecon.setParameters( actualParameters );
    REQUIRE_NOTHROW( recon.configure( ecalRecon ) );

    REQUIRE_NOTHROW( recon.produce( testEventBus ) );

    auto recHits{testEventBus.getCollection<EcalHit>( "EcalRecHits" )};

    //rec hits shouldn't lose any sim hits ??
    REQUIRE( pretendSimHits.size() <= recHits.size() );

    //sort the rec hits by ID
    std::sort( recHits.begin(), recHits.end(),
            []( const EcalHit &lhs , const EcalHit &rhs ) {
                return lhs.getID() < rhs.getID();
            });

    //now sim hits and rec hits are in the same order
    //  and rec hits have _at least_ as many hits as sim hits
    for ( unsigned int iHit = 0; iHit < pretendSimHits.size(); iHit++ ) {
        //argument to epsilon is maximum relative difference allowed
        //  right now, I set it to 0.025 (2.5% allowed difference)
        CHECK( recHits.at(iHit).getAmplitude() == Approx( pretendSimHits.at(iHit).getEdep() ).epsilon(0.025) );
    }
}
