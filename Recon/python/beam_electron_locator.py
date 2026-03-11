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
"""

from LDMX.Framework import Processor, processor


@processor("recon::BeamElectronLocator", "Recon")
class BeamElectronLocator(Processor):
    """The name is purely conventional to match the C++ class name for clarity

    Examples
    --------

    from LDMX.Recon.beamElectronLocator import BeamElectronLocator
    beamEleFinder= BeamElectronLocator('beamEleFinder')

    You can change the parameters after creating this object.
        beamEleFinder.my_parameter = 50

    Then you put your processor into the sequence of the process.
        p.sequence.append( beamEleFinder )
    """

    input_collection: str = "truthBeamElectronsTarget"
    input_pass_name: str = ""
    output_collection: str = "BeamElectronTruthInfo"
    granularity_x_mm: float = 2.5
    granularity_y_mm: float = 80.0 / 48.0
    min_granularity_mm: float = 0.1
    min_x_mm: float = -10.0
    max_x_mm: float = 10.0
    min_y_mm: float = -40.0
    max_y_mm: float = 40.0
    verbose: bool = False
