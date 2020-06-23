"""Primary Generator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

from LDMX.SimCore import simcfg

def gun( name ) :
    """New basic particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a ParticleGun without any of its parameters set
    """

    return simcfg.PrimaryGenerator( name , "ldmx::ParticleGun" )

def multi( name ) :
    """New multi particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a MultiParticleGun without any of its parameters set
    """

    return simcfg.PrimaryGenerator( name , "ldmx::MultiParticleGunPrimaryGenerator" )

def lhe( name , filePath ) :
    """New LHE file primary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to LHE file containing the primary vertices

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a LHEPrimaryGenerator with the input file given to it
    """

    sim = simcfg.PrimaryGenerator( name , "ldmx::LHEPrimaryGenerator" )
    sim.parameters[ "filePath" ] = filePath
    return sim

def completeReSim( name , filePath ) :
    """New complete re-simprimary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to ROOT file containing the SimParticles to re-simulate

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a RootCompleteReSim with the input file given to it
    """

    sim = simcfg.PrimaryGenerator( name , "ldmx::RootCompleteReSim" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "simParticleCollName" ] = "SimParticles"
    sim.parameters[ "simParticlePassName" ] = ""
    return sim

def ecalSP( name , filePath ) :
    """New ecal scoring planes primary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to ROOT file containing the EcalScoringPlanes to re-simulate

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a RootSimFromEcalSP with the input file given to it
    """

    sim = simcfg.PrimaryGenerator( name , "ldmx::RootSimFromEcalSP" )
    sim.parameters[ "filePath" ] = filePath
    sim.parameters[ "ecalSPHitsCollName" ] = "EcalScoringPlaneHits"
    sim.parameters[ "ecalSPHitsPassName" ] = ""
    sim.parameters[ "timeCutoff" ] = 50.
    return sim

def gps( name ) :
    """New general particle source

    Parameters
    ----------
    name : str
        name of new primary generator

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a GeneralParticleSource without any of its parameters set
    """
    return simcfg.PrimaryGenerator( name , "ldmx::GeneralParticleSource" )

def single_4gev_e_upstream_tagger() :
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by 
    the field and arrive at the target at approximately [0, 0, 0] (assuming 
    it's not smeared).

    The gun position below requires the particles to be fired at 4.5 degrees.
    The direction vector is calculated as follows: 
    
    dir_vector = [ sin(4.5) = .3138/4, 0, cos(4.5) = 3.9877/4 ] 
    
    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron 
    directly upstream of the tagger tracker.  

    """
    particle_gun = gun('single_4gev_e_upstream_tagger')
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ -27.926 , 0 , -700 ] # mm
    particle_gun.parameters[ 'direction' ] = [ 313.8 / 4000 , 0, 3987.7/4000 ]
    particle_gun.parameters[ 'energy'    ] = 4.0 # GeV

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
    particle_gun.parameters[ 'energy'    ] = 4.0 # GeV

    return particle_gun

def single_1pt2gev_e_upstream_tagger(): 
    """Configure a particle gun to fire a 1.2 GeV electron upstream of the tagger tracker.

    This is used to study the rejection of off energy electrons in the tagger
    tracker. The position and direction are set such that the electron will be 
    bent by  the field and arrive at the target at approximately [0, 0, 0]
    (assuming it's not smeared).

    The gun position below requires the particles to be fired at 11.011 degrees.
    The direction vector is calculated as follows: 
    
    dir_vector = [ sin(11.011) = .2292/1.2, 0, cos(11.011) = 1.1779/1.2 ] 

    
    Returns
    -------
    Instance of a particle gun configured to fire a single 1.2 Gev electron 
    directly upstream of the tagger tracker.  

    """
    particle_gun = gun( "single_1.2gev_e_upstream_tagger" )
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ -36.387, 0, -700 ] #mm
    particle_gun.parameters[ 'direction' ] = [ 0.2292 / 1.2 , 0, 1.1779 / 1.2 ] #unitless
    particle_gun.parameters[ 'energy'    ] = 1.2 #GeV
    
    return particle_gun


def single_8gev_e_upstream_tagger(): 
    """Configure a particle gun to fire a 8 GeV electron upstream of the tagger tracker.

    This is used to study the rejection of off energy electrons in the tagger
    tracker. The position and direction are set such that the electron will be 
    bent by  the field and arrive at the target at approximately [0, 0, 0]
    (assuming it's not smeared).

    The gun position below requires the particles to be fired at 2.5 degrees.
    The direction vector is calculated as follows: 
    
    dir_vector = [ 8.*sin(2.5) = .349, 0, 8.*cos(2.5) = 7.9924 ] 

    
    Returns
    -------
    Instance of a particle gun configured to fire a single 8 Gev electron 
    directly upstream of the tagger tracker.  

    """
    particle_gun = gun( "single_1.2gev_e_upstream_tagger" )
    particle_gun.parameters[ 'particle'  ] = 'e-'
    particle_gun.parameters[ 'position'  ] = [ -14.292, 0, -700 ] #mm
    particle_gun.parameters[ 'direction' ] = [ 0.349 / 8. , 0, 7.9924 / 8. ] #unitless
    particle_gun.parameters[ 'energy'    ] = 8.0 #GeV
    
    return particle_gun
