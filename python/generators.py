
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
# @return PrimaryGenerator of class ldmx::LHEPrimaryGenerator
#############################################################
def lhe( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::LHEPrimaryGenerator" )

#############################################################
# @function root
# @param name name of RootPrimaryGenerator
# @return PrimaryGenerator of class ldmx::RootPrimaryGenerator
#############################################################
def root( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::RootPrimaryGenerator" )

#############################################################
# @function stdhep
# @param name name of StdHepPrimaryGenerator
# @return PrimaryGenerator of class ldmx::StdHepPrimaryGenerator
#############################################################
def stdhep( name ) :
    print "stdhep PrimaryGenerator Not Implemented yet"
    quit()

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
def farUpstreamSingleElectron() :
    farUpstreamElectron = gun( "farUpstreamSingleElectron" )
    farUpstreamElectron.parameters[ 'particle'  ] = 'e-'
    farUpstreamElectron.parameters[ 'position'  ] = [ -27.926, 5, -700 ] #mm
    farUpstreamElectron.parameters[ 'direction' ] = [ 313.8 / 4000 , 0, 3987.7/4000 ] #unitless
    farUpstreamElectron.parameters[ 'energy'    ] = 4.0 #GeV
    return farUpstreamElectron
