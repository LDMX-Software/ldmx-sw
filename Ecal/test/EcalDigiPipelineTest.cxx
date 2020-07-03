/**
 * @file EcalDigiPipelineTest.cxx
 * @brief Test to make sure input sim energies are close to output rec energies
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Exception/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

#include "Framework/Event.h" //We need an event bus to pass through produce
#include "Framework/Process.h" //We need a dummy process to link the event bus to
#include "Event/EventDef.h" //Need event bus passengers
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

    std::map< std::string , std::any > requiredProcessParams, dummyAnalyzer;
    requiredProcessParams["passName"] = std::string("dummyProcess");
    requiredProcessParams["histogramFile"] = std::string();
    requiredProcessParams["logFileName"] = std::string();
    requiredProcessParams["maxTriesPerEvent"] = 1; 
    requiredProcessParams["maxEvents"] = 1; 
    requiredProcessParams["logFrequency"] = 1; 
    requiredProcessParams["termLogLevel"] = 4; 
    requiredProcessParams["fileLogLevel"] = 4; 
    requiredProcessParams["compressionSetting"] = 1; 
    requiredProcessParams["run"] = 1; 
    requiredProcessParams["skimDefaultIsKeep"] = false; 

    dummyAnalyzer["instanceName"] = std::string("dummy");
    dummyAnalyzer["className"] = std::string("ldmx::DummyAnalyzer");
    dummyAnalyzer["caloHitCollection"] = std::string("dummy");
    Parameters dummyConfig, dummyAna;
    dummyAna.setParameters(dummyAnalyzer);
    
    std::vector<Parameters> seq = { dummyAna };
    requiredProcessParams["sequence"] = seq;

    dummyConfig.setParameters(requiredProcessParams);

    Process dummyProcess( dummyConfig );
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
        currHit.setID( index++ );
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

    std::map< std::string , std::any > actualParameters;
    Parameters wrapperClass;

    EcalDigiProducer digis( "testDigis" , dummyProcess );
    actualParameters[ "gain" ] = 2000.;
    actualParameters[ "pedestal" ] = 1100.;
    actualParameters[ "noiseIntercept" ] = 700.;
    actualParameters[ "noiseSlope" ] = 25.;
    actualParameters[ "padCapacitance" ] = 0.1;
    actualParameters[ "nADCs" ] = 10; 
    actualParameters[ "iSOI"  ] = 0; 
    actualParameters[ "readoutThreshold" ] = 4.;
    actualParameters[ "makeConfigHists" ] = false;
    wrapperClass.setParameters( actualParameters );
    REQUIRE_NOTHROW( digis.configure( wrapperClass ) );

    REQUIRE_NOTHROW( digis.produce( testEventBus ) );

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
    wrapperClass.setParameters( actualParameters );
    REQUIRE_NOTHROW( recon.configure( wrapperClass ) );

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
