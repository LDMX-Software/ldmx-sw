

from LDMX.SimApplication import simcfg

def gun( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::ParticleGun" )

def multi( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::MultiParticleGunPrimaryGenerator" )

def lhe( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::LHEPrimaryGenerator" )
    sim.parameters[ "filePath" ] = filePath
    return sim

def completeReSim( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::RootCompleteReSim" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "simParticleCollName" ] = "SimParticles"
    sim.parameters[ "simParticlePassName" ] = ""
    return sim

def ecalSP( name , filePath ) :
    sim = simcfg.PrimaryGenerator( name , "ldmx::RootSimFromEcalSP" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "ecalSPHitsCollName" ] = "EcalScoringPlaneHits"
    sim.parameters[ "ecalSPHitsPassName" ] = ""
    sim.parameters[ "timeCutoff" ] = 50.
    return sim

def gps( name ) :
    return simcfg.PrimaryGenerator( name , "ldmx::GeneralParticleSource" )


def single_4gev_e_upstream_tagger() :
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at approximately [0, 0, 0] (assuming 
    it's not smeared).
    
    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron 
    directly upstream of the tagger tracker.  

    """

    particle_gun = gun('single_4gev_e_upstream_tagger')
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ -27.926 , 5 , -700 ] # mm
    particle_gun.parameters[ 'direction' ] = [ 313.8 / 4000 , 0, 3987.7/4000 ]
    particle_gun.parameters[ 'energy'    ] = 4.0000000 # GeV

    return particle_gun

def single_4gev_e_upstream_target() :
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at approximately [0, 0, 0] (assuming 
    it's not smeared).
    
    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron 
    directly upstream of the tagger tracker.  

    """

    particle_gun = gun('single_4gev_e_upstream_target')
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ 0., 0., -1.2 ] # mm
    particle_gun.parameters[ 'direction' ] = [ 0., 0., 1]
    particle_gun.parameters[ 'energy'    ] = 4.0000000 # GeV

    return particle_gun

def single_1.2gev_e_upstream_tagger(): 
    """Configure a particle gun to fire a 1.2 GeV electron upstream of the tagger tracker.

    This is used to study the rejection of off energy electrons in the tagger
    tracker. The position and direction are set such that the electron will be 
    bent by  the field and arrive at the target at approximately [0, 0, 0]
    (assuming it's not smeared).
    
    Returns
    -------
    Instance of a particle gun configured to fire a single 1.2 Gev electron 
    directly upstream of the tagger tracker.  

    """
    particle_gun = gun( "single_1.2gev_e_upstream_tagger" )
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ -36.387, 5, -700 ] #mm
    particle_gun.parameters[ 'direction' ] = [ 0.2292 / 1.2 , 0, 1.1779 / 1.2 ] #unitless
    particle_gun.parameters[ 'energy'    ] = 1.200000 #GeV
    return particle_gun
