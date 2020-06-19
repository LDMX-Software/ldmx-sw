"""Package to help configure the simulation

Defines a derived class from ldmxcfg.Producer
with several helpful member functions.
"""

from LDMX.Framework.ldmxcfg import Producer

class simulator(Producer):
    """A instance of the simulation configuration

    This class is derived from ldmxcfg.Producer and is mainly
    focused on providing helper functions that can be used instead
    of accessing the parameters member directly.

    The parameters that are lists ('preInitCommands', 'postInitCommands', 'actions', and 'generators')
    are initialized as empty lists so that we can append to them later.

    The ECal hit conbtibutions are enabled and compressed by default.

    Parameters
    ----------
    instance_name : str
        Name of this instance of the Simulator

    Attributes
    ----------
    generators : list of PrimaryGenerator
        Generators to use to make primaries
    detector : str
        Full path to detector description gdml (suggested to use setDetector)
    validate_detector : bool, optional
        Should we have Geant4 validate that the gdml is correctly formatted?
    runNumber : int
        Identifier for this run
    description : str
        Describe this run in a human-readable way
    scoringPlanes : str, optional
        Full path to the scoring planes gdml (suggested to use setDetector)
    randomSeeds : list of int, optional
        Random seeds to pass to Geant4
    beamSpotSmear : list of float, optional
        2 (x,y) or 3 (x,y,z) widths to smear ALL primary vertices by [mm]
    enableHitContribs : bool, optional
        Should the simulation save contributions to Ecal sim hits?
    compressHitContribs : bool, optional
        Should the simulation compress contributions to Ecal sim hits by PDG ID?
    preInitCommands : list of str, optional
        Geant4 commands to run before the run is initialized
    postInitCommands : list of str, optional
        Geant4 commands to run after run is initialized (but before run starts)
    actions : list of UserAction, optional
        Special User-defined actions to take during the simulation
    biasing_enabled : bool, optional
        Should the simulation configure and use biasing?
    biasing_process : str, optional
        Geant4 process to bias
    biaising_volume : str, optional
        Geant4 volume to bias inside of
    biasing_particle : str, optional
        Geant4 particle to bias
    biasing_all : bool, optional
        Bias all particles of the input type (only used by some biasing operators)
    biasing_incident : bool, optional
        Bias only the primary particle of the input type (only used by some biasing operators)
    biasing_disableEMBiasing : bool, optional
        Turn off down-biasing of normal EM shower (only used by some biasing operators)
    biasing_threshold : float, optional
        Minimum energy [MeV] for a particle to be biased (only used by some biasing operators)
    biasing_factor : int, optional
        Factor to bias process by
    APrimeMass : float, optional
        Mass of A' to simulation [MeV]
    darkbrem_madgraphfilepath : str, optional
        Full path to LHE file to use as dark brem vertices
    darkbrem_method : int, optional
        Integer flag to decide how to interpret dark brem vertices
    darkbrem_globalxsecfactor : float, optional
        Bias the dark brem process everywhere by this factor
    logging_prefix : str, optional
        Prefix to prepend any Geant4 logging files
    rootPrimaryGenUseSeed : bool, optional
        Use the seed stored in the EventHeader for random generation
    verbosity : int, optional
        Verbosity level to print
    """

    def __init__(self, instance_name ) :
        super().__init__( instance_name , "ldmx::Simulator" )

        #######################################################################
        # Required Parameters
        self.generators = [ ]
        self.detector = ''
        self.runNumber = -1
        self.description = ''

        #######################################################################
        # Optional Parameters (with helpful defaults)
        self.scoringPlanes = ''
        self.randomSeeds = [ ] 
        self.beamSpotSmear = [ ]
        self.enableHitContribs   = True
        self.compressHitContribs = True
        self.preInitCommands = [ ]
        self.postInitCommands = [ ]
        self.actions = [ ]
        self.logging_prefix = ''
        self.rootPrimaryGenUseSeed = False
        self.validate_detector = False
        self.verbosity = 0

        # Biasing stuff
        self.biasing_enabled = False
        self.biasing_process = ''
        self.biasing_volume = ''
        self.biasing_particle = ''
        self.biasing_all = True
        self.biasing_incident = True
        self.biasing_disableEMBiasing = False
        self.biasing_threshold = 0.
        self.biasing_factor = 1

        # Dark Brem stuff
        self.APrimeMass = 0. #required if want to use dark brem [MeV]
        self.darkbrem_madgraphfilepath = '' #required if want to use dark brem
        self.darkbrem_method = 0
        self.darkbrem_globalxsecfactor = 1.

        # add necessary library to the list to load
        #   requires a process object to have been defined
        from LDMX.SimApplication import include
        include.library()

    def setDetector(self, det_name , include_scoring_planes = False ) :
        """Set the detector description with the option to include the scoring planes

        Parameters
        ----------
        det_name : str
            name of a detector in the Detectors module
        include_scoring_planes : bool
            True if you want to import and use scoring planes

        See Also
        --------
        LDMX.Detectors.makePath for definitions of the path making functions.
        """

        from LDMX.Detectors import makePath as mP
        self.detector = mP.makeDetectorPath( det_name )
        if include_scoring_planes :
            self.scoringPlanes = mP.makeScoringPlanesPath( det_name )

    def biasingOn(self,yes=True):
        """Turn biasing on in the simulation

        Parameters
        ----------
        yes : bool
            we should turn the biasing on
        """

        self.biasing_enabled = yes

    def biasingConfigure(self,process,volume,threshold,factor,allPtl=True,incidentOnly=True,disableEMBiasing=False) :
        """Configure the biasing for this simulation
        
        The particle to bias is chosen by a python dictionary from available
        processes. In order to bias a process in Geant4 we need to define a XsecBiasingOperator.
        This limits the processes we can bias to the three that we have biasing operators for.

        Parameters
        ----------
        process : str
            Geant4 name of the process to bias
        volume : str
            Name of volume to bias within
        threshold : float
            Minimum energy threshold [MeV] for a particle to be biased
        factor : float
            Factor to bias by
        allPtl : bool
            True if all particles that can use the input process are to be biased
        incidentOnly : bool
            True if only the incident particle should be biased
        disableEMBiasing : bool
            True if we should turn off the EM down-biasing
        """

        processToParticle = {
            'eDBrem'          : 'e-',
            'photonNuclear'   : 'gamma',
            'electronNuclear' : 'e-'
            }

        self.biasing_process = process
        self.biasing_volume = volume
        self.biasing_particle
        self.biasing_all = allPtl
        self.biasing_incident = incidentOnly
        self.biasing_disableEMBiasing = disableEMBiasing
        self.biasing_threshold = threshold
        self.biasing_factor = factor

    def darkBremOn(self,ap_mass,lhe_file_path,method=1,globalxsecfactor=1.) :
        """Configure the dark brem simulation

        This method implicitly activates the dark brem in the simulation because
        a LHE file is passed. If this LHE file does not exist, an exception will
        be thrown by the dark brem process.

        Parameters
        ----------
        ap_mass : float
            Mass of A' [MeV] to simulate (must match the mass in the LHE file)
        lhe_file_path : str
            Path to the LHE file containing dark brem vertices
        method : int, optional
            Tell the dark brem simulation how to interpret the vertices
        globalxsecfactor : float, optional
            Bias the dark brem process globally by the input factor
        """

        self.APrimeMass = ap_mass
        self.darkbrem_madgraphfilepath = lhe_file_path
        self.darkbrem_method = method
        self.darkbrem_globalxsecfactor = globalxsecfactor
