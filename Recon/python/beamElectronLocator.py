"""Configuration for beam electron locator 

Attributes:
-------------
input_collection : string
    Name of the input collection, should be one that holds sim hits 
input_pass_name : string
    Pass name of the input collection
output_collection : string
    Name of the output collection, holding the condensed list of beam electron locations
granularity_X_mm : float
    The expected actual resolution or the bar width in mm of the TS in the X direction
granularity_Y_mm : float
    The expected actual resolution or the bar width in mm of the TS in the Y direction
min_granularity_mm : float
    The maximum distance between SimHits that are grouped together as one beam electron
verbose : boolean
    If set to true, more information is spit out to either Info or Debug log level

# We need the ldmx configuration package to construct the processor objects
"""

from LDMX.Framework import ldmxcfg

class BeamElectronLocator(ldmxcfg.Producer) :
    """The name is purely conventional to match the C++ class name for clarity

    Examples
    --------

    from LDMX.Recon.beamElectronLocator import BeamElectronLocator
    beamEleFinder= BeamELectronLocator('beamEleFinder')

    You can change the parameters after creating this object.
        beamEleFinder.my_parameter = 50

    Then you put your processor into the sequence of the process.
        p.sequence.append( beamEleFinder )
    """

    def __init__(self, name ):
        super().__init__( name , "recon::BeamElectronLocator" , 'Recon' )

        self.input_collection = "truthBeamElectronsTarget"
        self.input_pass_name  = ""
        self.output_collection = "BeamElectronTruthInfo"
        self.granularity_X_mm = 20./8.
        self.granularity_Y_mm = 80./48.
        self.min_granularity_mm = 0.1
        self.min_X_mm = -10.
        self.max_X_mm = 10.
        self.min_Y_mm = -40.
        self.max_Y_mm = 40.
        self.verbose = False

