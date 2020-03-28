
##################################################################################
# @file generators.py
# Define some helpful functions and standardized generators for the simulation
#
# @author Tom Eichlersmith, University of Minnesota
##################################################################################

from LDMX.SimApplication import simcfg

#############################################################
# @function gun
# @param name name of ParticleGun
# @return PrimaryGenerator of class ldmx::ParticleGun
#############################################################
def gun( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::ParticleGun" )

#############################################################
# @function multi
# @param name name of MultiParticleGunPrimaryGenerator
# @return PrimaryGenerator of class ldmx::MultiParticleGunPrimaryGenerator
#############################################################
def multi( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::MultiParticleGunPrimaryGenerator" )

#############################################################
# @function lhe
# @param name name of LHEPrimaryGenerator
# @param filePath LHE file to use
# @return PrimaryGenerator of class ldmx::LHEPrimaryGenerator with filePath parameter set
#############################################################
def lhe( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::LHEPrimaryGenerator" )
    sim.parameters[ "filePath" ] = filePath
    return sim

#############################################################
# @function completeReSim
# @param name name of RootCompleteReSim
# @param filePath root file to re-sim
# @return PrimaryGenerator of class ldmx::RootCompleteReSim with parameters set to reasonable defaults
#############################################################
def completeReSim( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::RootCompleteReSim" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "simParticleCollName" ] = "SimParticles"
    sim.parameters[ "simParticlePassName" ] = ""
    return sim

#############################################################
# @function ecalSP
# @param name name of RootSimFromEcalSP
# @param filePath root file to re-sim
# @return PrimaryGenerator of class ldmx::RootSimFromEcalSP with parameters set to reasonable defaults
#############################################################
def ecalSP( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::RootSimFromEcalSP" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "ecalSPHitsCollName" ] = "EcalScoringPlaneHits"
    sim.parameters[ "ecalSPHitsPassName" ] = ""
    sim.parameters[ "timeCutoff" ] = 50.
    return sim

#############################################################
# @function stdhep
# @param name name of StdHepPrimaryGenerator
# @return PrimaryGenerator of class ldmx::StdHepPrimaryGenerator
#############################################################
import sys
def stdhep( name ) :
    print "stdhep PrimaryGenerator Not Implemented yet"
    sys.exit(1)

#############################################################
# @function gps
# @param name name of GeneralParticleSource
# @return PrimaryGenerator of class ldmx::GeneralParticleSource
#############################################################
def gps( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::GeneralParticleSource" )

#############################################################
# @function farUpstreamSingleElectron
# @return a ParticleGun with a single 4GeV electron fired from far upstream of target
#############################################################
def farUpstreamSingle4GeVElectron() :
    farUpstreamElectron = gun( "farUpstreamSingleElectron" )
    farUpstreamElectron.parameters[ 'particle'  ] = 'e-'
    farUpstreamElectron.parameters[ 'position'  ] = [ -27.926, 5, -700 ] #mm
    farUpstreamElectron.parameters[ 'direction' ] = [ 313.8 / 4000 , 0, 3987.7/4000 ] #unitless
    farUpstreamElectron.parameters[ 'energy'    ] = 4.0000000 #GeV
    return farUpstreamElectron
