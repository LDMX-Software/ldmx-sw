/**
 * @file EcalDigiPipelineTest.cxx
 * @brief Test to make sure input sim energies are close to output rec energies
 * @author Tom Eichlersmith, University of Minnesota
 */

#include "Framework/catch.hpp" //for TEST_CASE, REQUIRE, and other Catch2 macros

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
 *  Close is defined right now to be a maximum of 5% relative difference.
 *
 * @TODO still need to expand to multiple contribs in a single sim hit
 * @TODO check ability to discard noise / low energy sim hits
 * @TODO check layer weights are being calculated correctly somehow
 */
TEST_CASE( "Ecal Digi Pipeline test" , "[Ecal][functionality]" ) {

    using namespace ldmx;

    auto dummyProcess{Process::getDummy()};
    Event testEventBus( "testEcalDigiPipeline" );

    std::vector<SimCalorimeterHit> pretendSimHits;

    //fill pretend sim hits with a range of simulated energies
    double maxEnergy  = 80. ; //MeV
    double energyStep = 0.1 ; //MeV
    double currEnergy = 0.5 ; //minEnergy MeV <-- needs to be above readout threshold (after internal EcalDigi's calculation)
    int cell(0), module(0), layer(0);
    std::map< EcalID , double > correct_energies; //for ease of comparison later
    while ( currEnergy < maxEnergy ) {
        //TODO test several contribs
        //  right now, not a big deal because we just do an energy weight average anyways
        SimCalorimeterHit currHit;
        //distribute indices across distince IDs
        //  need to be careful to not exceed max count of subparts
        EcalID id(layer,module,cell);
        currHit.setID( id.raw() );
        currHit.addContrib(
                -1 //incidentID
                , -1 // trackID
                , 0 // pdg ID
                , currEnergy // edep
                , 1. //time
                );
        correct_energies[ id ] = currEnergy; //for ease of comparison layer
        pretendSimHits.push_back( currHit );

        currEnergy += energyStep;
        cell++;
        if ( cell > 300 ) {
            cell = 0;
            module++;
        }
        if ( module > 7 ) {
            module = 0;
            layer++;
        }
    }

    testEventBus.add( "EcalSimHits" , pretendSimHits ); //needs to be correct collection name

    EcalDigiProducer digis( "testDigis" , dummyProcess );

    Parameters hgcroc;
    hgcroc.addParameter( "rateUpSlope" ,  -0.345);
    hgcroc.addParameter( "timeUpSlope" , 70.6547);
    hgcroc.addParameter( "rateDnSlope" , 0.140068);
    hgcroc.addParameter( "timeDnSlope" , 87.7649);
    hgcroc.addParameter( "timePeak"    , 77.732);
    hgcroc.addParameter( "pedestal" , 50. );
    hgcroc.addParameter( "clockCycle" , 25.0 );
    hgcroc.addParameter( "measTime" , 0. );
    hgcroc.addParameter( "timingJitter" , 25. / 100. );
    hgcroc.addParameter( "readoutPadCapacitance" , 0.1 );
    hgcroc.addParameter( "maxADCRange" , 320. );
    hgcroc.addParameter( "nADCs" , 10 );
    hgcroc.addParameter( "iSOI"  , 0 );
    hgcroc.addParameter( "totMax" , 200. );
    hgcroc.addParameter( "drainRate" , 4000. / 0.1 / 200. );
    double gain = 320. / 0.1 / 1024;
    hgcroc.addParameter( "gain" , gain );
    double electrons_to_voltage = (0.162/1000)/0.1;
    double noiseRMS = (700. + 25*0.1)*electrons_to_voltage;
    hgcroc.addParameter( "noiseRMS"         , noiseRMS );
    hgcroc.addParameter( "readoutThreshold" , gain*50. + 4 *noiseRMS );
    int n_electrons_per_mip = 37000;
    hgcroc.addParameter( "toaThreshold"     , gain*50. + 5*n_electrons_per_mip*electrons_to_voltage );
    hgcroc.addParameter( "totThreshold"     , gain*50. + 50*n_electrons_per_mip*electrons_to_voltage );

    Parameters ecalDigi;
    ecalDigi.addParameter( "randomSeed" , 1);
    ecalDigi.addParameter( "nEcalLayers" , 34 );
    ecalDigi.addParameter( "nModulesPerLayer" , 7 );
    ecalDigi.addParameter( "nCellsPerModule" , 397 );
    double mip_si_energy = 0.130;
    ecalDigi.addParameter( "MeV" , (1./mip_si_energy)*n_electrons_per_mip*electrons_to_voltage );
    ecalDigi.addParameter( "hgcroc" , hgcroc );

    REQUIRE_NOTHROW( digis.configure( ecalDigi ) );

    REQUIRE_NOTHROW( digis.produce( testEventBus ) );
    
    Parameters hexReadout;
    hexReadout.addParameter("gap", 1.5);
    hexReadout.addParameter("moduleMinR", 85.0);
    hexReadout.addParameter("ecalFrontZ", 240.5);
    hexReadout.addParameter("nCellRHeight", 35.3);
    hexReadout.addParameter("verbose", 0);
    std::vector<double> layerZPositions = {
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
    };
    hexReadout.addParameter("layerZPositions", layerZPositions);

    EcalRecProducer recon( "testRecon" , dummyProcess);

    Parameters ecalRecon;
    ecalRecon.addParameter( "digiCollName" , std::string("EcalDigis"));
    ecalRecon.addParameter( "digiPassName" , std::string());
    ecalRecon.addParameter( "secondOrderEnergyCorrection" , 4000./4010.);
    std::vector<double> layerWeights = {
        1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
        17.364, 8.990
    };
    ecalRecon.addParameter( "layerWeights" , layerWeights);
    ecalRecon.addParameter( "hexReadout"   , hexReadout);
    ecalRecon.addParameter( "hgcroc"       , hgcroc );
    ecalRecon.addParameter( "mV"           , mip_si_energy / (n_electrons_per_mip*electrons_to_voltage) );
    ecalRecon.addParameter( "mipSiEnergy"  , mip_si_energy );

    REQUIRE_NOTHROW( recon.configure( ecalRecon ) );

    REQUIRE_NOTHROW( recon.produce( testEventBus ) );

    auto recHits{testEventBus.getCollection<EcalHit>( "EcalRecHits" )};

    for ( const auto& hit : recHits ) {
        EcalID id(hit.getID());
        if ( correct_energies.count(id) > 0 ) {
            //not a noise hit
            //argument to epsilon is maximum relative difference allowed
            //  right now, I set it to 0.05 (5% allowed difference)
            CHECK_FALSE( hit.isNoise() );
            CHECK( hit.getAmplitude() == Approx( correct_energies.at(id) ).epsilon(0.05) );
            correct_energies.erase(id);
        } else {
            //should be flagged as noise
            CHECK( hit.isNoise() );
        }
    }

    // did we lose any energies during the processing?
    //  all of the correct eneriges should have been visited and erased in the above loop
    CHECK( correct_energies.size() == 0 );
}
