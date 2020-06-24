"""Primary Generator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

from LDMX.SimCore import simcfg

class gun(simcfg.PrimaryGenerator) :
    """New basic particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Attributes
    ----------
    time : float, optional
        Time to shoot from [ns]
    verbosity : int, optional
        Verbosity flag for this generator
    particle : str
        Geant4 particle name to shoot
    energy : float
        Energy of particle to shoot [GeV]
    position : list of float
        Position to shoot from [mm]
    direction : list of float
        Unit vector direction to shoot from

    Examples
    --------
        myGun = gun( 'myGun' )
        myGun.particle = 'e-'
        myGun.energy = 4.0
        myGun.direction = [ 0., 0., 1. ]
        myGun.position = [ 0., 0., 0. ]
    """

    def __init__(self, name ) :
        super().__init__( name , "ldmx::ParticleGun" )

        self.time = 0.
        self.verbosity = 0
        self.particle = ''
        self.energy = 0.
        self.position = [ ]
        self.direction = [ ]

class multi(simcfg.PrimaryGenerator) :
    """New multi particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Attributes
    ----------
    enablePoisson : bool, optional
        Poisson-distribute number of particles?
    vertex : list of float
        Position to shoot particle(s) from [mm]
    momentum : list of float
        3-momentum to give particle(s) in [MeV]
    nParticles : int, optional
        Number of particles to shoot (or average of Poisson distribution)
    pdgID : int
        PDG ID of particle(s) to shoot
    """

    def __init__(self,name) :
        super().__init__(name,'ldmx::MultiParticleGunPrimaryGenerator')

        #turn off Poisson by default
        self.enablePoisson = False
        self.vertex = [ ]
        self.momentum = [ ]
        self.nParticles = 1
        self.pdgID = 0


class lhe(simcfg.PrimaryGenerator) :
    """New LHE file primary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to LHE file containing the primary vertices
    """

    def __init__(self,name,filePath):
        super().__init__(name,'ldmx::LHEPrimaryGenerator')

        self.filePath = filePath

class completeReSim(simcfg.PrimaryGenerator) :
    """New complete re-simprimary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    file_path : str
        path to ROOT file containing the SimParticles to re-simulate

    Attributes
    ----------
    collection_name : str
        Name of SimParticles collection to re-sim
    pass_name : str
        Pass name of SimParticles to re-sim
    """

    def __init__(self,name,file_path) :
        super().__init__(name,'ldmx::RootCompleteReSim')
        
        self.filePath = file_path
        self.collection_name = 'SimParticles'
        self.pass_name = ''

class ecalSP(simcfg.PrimaryGenerator) :
    """New ecal scoring planes primary generator

    Sets the collection name, pass name, and time cutoff
    to reasonable defaults.

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to ROOT file containing the EcalScoringPlanes to re-simulate


    Attributes
    ----------
    collection_name : str, optional
        Name of EcalScoringPlaneHits collection to re-sim
    pass_name : str, optional
        Pass name of EcalScoringPlaneHits to re-sim
    time_cutoff : float, optional
        Maximum time of scoring plane hit to still re-sim [ns]
    """

    def __init__(self,name,filePath) :
        super().__init__( name , 'ldmx::RootSimFromEcalSP' )

        self.filePath = filePath
        self.collection_name = 'EcalScoringPlaneHits'
        self.pass_name = ''
        self.time_cutoff = 50.

class gps(simcfg.PrimaryGenerator) :
    """New general particle source

    The input initialization commands are run in the order that they are listed.

    Parameters
    ----------
    name : str
        name of new primary generator
    initCommands : list of strings
        List of Geant4 commands to initialize this GeneralParticleSource

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a GeneralParticleSource with the passed initialization commands

    Examples
    --------
        myGPS = gps( 'myGPS' , [
            "/gps/particle e-",
            "/gps/pos/type Plane",
            "/gps/pos/shape Square",
            "/gps/pos/centre 0 0 0 mm",
            "/gps/pos/halfx 40 mm",
            "/gps/pos/halfy 80 mm",
            "/gps/ang/type cos",
            "/gps/ene/type Lin",
            "/gps/ene/min 3 GeV",
            "/gps/ene/max 4 GeV",
            "/gps/ene/gradient 1",
            "/gps/ene/intercept 1"
            ] )
    """

    def __init__(self,name,initCommands) :
        super().__init__(name,'ldmx::GeneralParticleSource')
        self.initCommands = initCommands

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
    particle_gun.particle = 'e-'
    particle_gun.position = [ -27.926 , 0 , -700 ] # mm
    import math
    theta = math.radians(4.5)
    particle_gun.direction = [ math.sin(theta) , 0, math.cos(theta) ] #unitless
    particle_gun.energy = 4.0 # GeV

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
    particle_gun.particle = 'e-' 
    particle_gun.position = [ 0., 0., -1.2 ]  # mm
    particle_gun.direction = [ 0., 0., 1] 
    particle_gun.energy = 4.0 # GeV

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
    particle_gun.particle = 'e-' 
    particle_gun.position = [ -36.387, 0, -700 ] #mm
    import math
    theta = math.radians(11.011)
    particle_gun.direction = [ math.sin(theta) , 0, math.cos(theta) ] #unitless
    particle_gun.energy = 1.2 #GeV
    
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
    particle_gun.particle = 'e-' 
    particle_gun.position = [ -14.292, 0, -700 ] #mm
    import math
    theta = math.radians(2.5)
    particle_gun.direction = [ math.sin(theta) , 0, math.cos(theta) ] #unitless
    particle_gun.energy = 8.0 #GeV
    
    return particle_gun
