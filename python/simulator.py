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
    """

    def __init__(self, instance_name ) :
        super().__init__( instance_name , "ldmx::Simulator" )

        # make sure the lists are the correct type
        self.parameters['preInitCommands' ] = [ ]
        self.parameters['postInitCommands'] = [ ]
        self.parameters['actions'         ] = [ ]
        self.parameters['generators'      ] = [ ]

        # turn on default ECal Hit behavior
        self.enableHitContribs()
        self.compressHitContribs()

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
        self.parameters['detector'] = mP.makeDetectorPath( det_name )
        if include_scoring_planes :
            self.parameters['scoringPlanes'] = mP.makeScoringPlanesPath( det_name )

    def setRunNumber(self, num ) :
        """Set the run number for this simulation

        Parameters
        ----------
        num : int
            run number for this simulation
        """

        self.parameters['runNumber'] = num

    def setDescription(self,desc) :
        """Set the description of this simulation

        Parameters
        ----------
        desc : str
            description of this simulation
        """

        self.parameters['description'] = desc

    def setRandomSeeds(self,seeds) :
        """Set the random seeds for this simulation

        Parameters
        ----------
        seeds : list of ints
            list of random seeds to pass to Geant4
        """

        self.parameters['randomSeeds'] = seeds

    def setBeamSpotSmear(self,smear) :
        """Set how much to smear primary vertices by

        This smearing affects ALL primary vertices across
        ALL primary generators.

        Parameters
        ----------
        smear : list of floats
            2 (x,y) or 3 (x,y,z) of widths to smear vertices by in mm
        """

        self.parameters['beamSpotSmear'] = smear

    def enableHitContribs(self,yes=True):
        """Allow for the ECal to store the different contributors to its sim hits

        If you aren't doing ECal studies and don't care about the ECal digi
        pipeline, it might help you save space and time if you call this
        with False as the input.

        Parameters
        ----------
        yes : bool
            we should allow ECal to store the hit contributors
        """

        self.parameters['enableHitContribs'] = yes

    def compressHitContribs(self,yes=True) : 
        """Compress the ECal hit contribs by PDG ID

        If you are really focused on ECal studies and don't care
        about space or time requirements, you can call this
        with False as the input.

        Parameters
        ----------
        yes : bool
            we should compress the hit contribs by PDG ID
        """

        self.parameters['compressHitContribs'] = yes

    def biasingOn(self,yes=True):
        """Turn biasing on in the simulation

        Parameters
        ----------
        yes : bool
            we should turn the biasing on
        """

        self.parameters['biasing.enabled'] = yes

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

        self.parameters['biasing.process'         ] = process
        self.parameters['biasing.volume'          ] = volume
        self.parameters['biasing.particle'        ] = processToParticle[process]
        self.parameters['biasing.all'             ] = allPtl
        self.parameters['biasing.incident'        ] = incidentOnly
        self.parameters['biasing.disableEMBiasing'] = disableEMBiasing
        self.parameters['biasing.threshold'       ] = threshold
        self.parameters['biasing.factor'          ] = factor

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

        self.parameters['APrimeMass'] = ap_mass
        self.parameters['darkbrem.madgraphfilepath'] = lhe_file_path
        self.parameters['darkbrem.method'          ] = method
        self.parameters['darkbrem.globalxsecfactor'] = globalxsecfactor

    def preInitCommands(self) :
        """Access the pre- run initialization Geant4 commands list"""
        return self.parameters['preInitCommands']

    def postInitCommands(self) :
        """Access the post- run initialization Geant4 commands list"""
        return self.parameters['postInitCommands']

    def actions(self) :
        """Access the list of UserActions attached to this simulation"""
        return self.parameters['actions']

    def generators(self) :
        """Access the list of PrimaryGenerators attached to this simulation"""
        return self.parameters['generators']
