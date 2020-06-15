"""Primary Generator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

from LDMX.SimApplication import simcfg

class gun(simcfg.PrimaryGenerator) :
    """New basic particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator

    Examples
    --------
        myGun = gun( 'myGun' )
    """

    def __init__(self, name ) :
        super().__init__( name , "ldmx::ParticleGun" )

        self.time(0.)
        self.verbosity(0)

    def verbosity( self , v ) :
        """Set the verobisty for this particle gun

        Parameters
        ----------
        v : int
            verbosity level
        """

        self.parameters['verbosity'] = v

    def particle( self , p ) :
        """Set the particle to shoot

        Parameters
        ----------
        p : str
            Geant4 name of particle to shoot (e.g. 'e-' or 'gamma')

        Examples
        --------
            myGun.particle( 'e-' )
        """

        self.parameters['particle'] = p

    def energy( self , e ) :
        """Set the energy of the particle to shoot
        
        Parameters
        ----------
        e : float
            Energy of particle in GeV

        Examples
        --------
            myGun.energy( 4.0 )
        """

        self.parameters['energy'] = e

    def position( self , p ) :
        """Position to shoot particle from

        Parameters
        ----------
        p : list of floats
            3-vector position to shoot from in mm

        Examples
        --------
            myGun.position( [ 0. , 0. , 1. ] )
        """

        self.parameters['position'] = p

    def time( self , t ) :
        """Time at which to shoot this gun

        Parameters
        ----------
        t : float
            Time to shoot this gun in ns

        Examples
        --------
            myGun.time( 25. )
        """

        self.parameters['time'] = t

    def direction( self , d ) :
        """Unit direction vector to shoot this particle in

        Parameters
        ----------
        d : list of floats
            Unit direction vector to shoot this particle in

        Examples
        --------
            myGun.direction( [ 0., 0., 1. ] )
        """

        self.parameters['direction'] = d

class multi(simcfg.PrimaryGenerator) :
    """New multi particle gun primary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    """

    def __init__(self,name) :
        super().__init__(self,name,'ldmx::MultiParticleGunPrimaryGenerator')

        #turn off Poisson by default
        self.enablePoisson(False)

    def vertex(self,v) :
        """Set the vertex for this gun

        Parameters
        ----------
        v : list of floats
            3-vector starting position in mm
        
        Examples
        --------
            myMulti.vertex( [ 0., 0., 0. ] )
        """

        self.parameters['vertex'] = v

    def momentum(self,p) :
        """Set the momentum of the particles in this gun

        Parameters
        ----------
        p : list of floats
            3-vector momentum in MeV

        Examples
        --------
            myMulti.momentum([0.,0.,4000.])
        """

        self.parameters['momentum'] = p

    def nParticles(self,n) :
        """Set the number of particles to shoot

        If Poisson is enabled, then this is the average of the Poisson distribution.
        If Poisson is not enabled, then the gun shoots this many particles every event.

        Parameters
        ----------
        n : int
            Number of particles

        Examples
        --------
            myMulti.nParticles(2)
        """

        self.parameters['nParticles'] = n

    def pdgID(self,i) :
        """Set the ID of the particle to shoot

        Parameters
        ----------
        i : int
            PDG ID of the particle(s) to shoot

        Examples
        --------
            myMulti.pdgID(11)
        """

        self.parameters['pdgID'] = i

    def enablePoisson(self,yes=True) :
        """Turn on Poisson-distribution of number of particles

        Parameters
        ----------
        yes : bool
            We should Poisson-distribute the number of particles

        Examples
        --------
            myMulti.enablePoisson()
        """

        self.parameters['enablePoisson'] = yes


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

class completeReSim(simcfg.PrimaryGenerator) :
    """New complete re-simprimary generator

    Parameters
    ----------
    name : str
        name of new primary generator
    file_path : str
        path to ROOT file containing the SimParticles to re-simulate

    Examples
    --------
        myComplete = completeReSim('myComplete','old_sim_file.root')
    """

    def __init__(self,name,file_path) :
        super().__init__(name,'ldmx::RootCompleteReSim')
        
        self.parameters['filePath'] = file_path
        self.collection_name('SimParticles')
        self.pass_name('')

    def collection_name(self,name) :
        """Set the SimParticle collection name

        Parameters
        ----------
        name : str
            Collection name to use for re-sim

        Examples
        --------
            myComplete.collection_name('SimParticles')
        """

    def pass_name(self,name) :
        """Set the SimParticle pass name

        Parameters
        ----------
        name : str
            Pass name to use for re-sim

        Examples
        --------
            myComplete.pass_name('special')
        """

class ecalSP( name , filePath ) :
    """New ecal scoring planes primary generator

    Sets the collection name, pass name, and time cutoff
    to reasonable defaults.

    Parameters
    ----------
    name : str
        name of new primary generator
    filePath : str
        path to ROOT file containing the EcalScoringPlanes to re-simulate

    Examples
    --------
        mySP = ecalSP( 'mySP' , 'old_sim_file.root' )
    """

    def __init__(self,name,filePath) :
        super().__init__( name , 'ldmx::RootSimFromEcalSP' )

        self.parameters[ "filePath" ] = filePath
        self.collection_name('EcalScoringPlaneHits')
        self.pass_name('')
        self.time_cutoff(50.)

    def collection_name(self,name) :
        """Set the collection name to use

        Parameters
        ----------
        name : str
            Name of collection

        Examples
        --------
            mySP.collection_name('EcalScoringPlanes')
        """

        self.parameters[ "ecalSPHitsCollName" ] = name

    def pass_name(self,name) :
        """Set the pass name to use

        Parameters
        ----------
        name : str
            Name of pass

        Examples
        --------
            mySP.pass_name('specialpass')
        """

        self.parameters[ "ecalSPHitsPassName" ] = name

    def time_cutoff(self,t) :
        """Set maximum time of scoring plane hits to consider

        Parameters
        ----------
        t : float
            Maximum time in ns

        Examples
        --------
            mySP.time_cutoff(100.)
        """

        self.parameters[ "timeCutoff" ] = t

def gps( name , initCommands ) :
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

    sim = simcfg.PrimaryGenerator( name , 'ldmx::GeneralParticleSource' )
    sim.parameters['initCommands'] = initCommands
    return sim

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
    particle_gun.particle('e-')
    particle_gun.position([ -27.926 , 0 , -700 ]) # mm
    import math
    theta = math.radians(4.5)
    particle_gun.direction([ math.sin(theta) , 0, math.cos(theta) ]) #unitless
    particle_gun.energy(4.0) # GeV

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
    particle_gun.particle( 'e-' )
    particle_gun.position( [ 0., 0., -1.2 ] ) # mm
    particle_gun.direction( [ 0., 0., 1] )
    particle_gun.energy( 4.0 )# GeV

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
    particle_gun.particle( 'e-' )
    particle_gun.position( [ -36.387, 0, -700 ] )#mm
    import math
    theta = math.radians(11.011)
    particle_gun.direction( [ math.sin(theta) , 0, math.cos(theta) ] )#unitless
    particle_gun.energy( 1.2 )#GeV
    
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
    particle_gun.particle( 'e-' )
    particle_gun.position( [ -14.292, 0, -700 ] )#mm
    import math
    theta = math.radians(2.5)
    particle_gun.direction( [ math.sin(theta) , 0, math.cos(theta) ] )#unitless
    particle_gun.energy( 8.0 )#GeV
    
    return particle_gun
