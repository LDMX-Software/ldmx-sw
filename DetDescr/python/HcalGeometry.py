"""Configuration for HcalGeometry"""

class HcalReadoutGeometry() :
    """Configuration for HcalGeometry for a specific geometry

    Attributes
    ----------
    detectors_valid : array of strings
        Regular expressions identifying which detectors are valid for this geometry
    """

    def __init__(self) :

        # parameters that must align with the geometry
        self.detectors_valid = [ ]
        self.hcalThicknessScint = 0.
        self.hcalWidthScint = 0.
        self.hcalZeroLayer = []
        self.hcalZeroStrip = []
        self.hcalLayerThickness = []
        self.hcalNLayers = []
        self.hcalNStrips = []
        self.hcalHalfTotalWidthBack = 0.
        self.verbose = 0

    def __str__(self) :
        """Stringify this configuration class"""

        s = 'HcalReadoutGeometry'
        return s

class HcalGeometry() :
    """Container for the various geometries

    """

    def __init__(self):
        self.make_v12()
    
    def make_v12(self) :
        """Create the HcalGeometry with the v12 geometry parameters

        Only sets parameters that must align with the gdml
        """
        self.v12=HcalReadoutGeometry()

        self.v12.hcalThicknessScint = 20.0
        self.v12.hcalWidthScint = 50.0
        self.v12.hcalZeroLayer = [220.+600.,600./2,600./2,600./2,600./2]
        self.v12.hcalZeroStrip = [3100./2,220.,220.,220.,220.]
        self.v12.hcalLayerThickness = [25. + self.v12.hcalThicknessScint + 2*2.,
                                       20. + self.v12.hcalThicknessScint + 2*2., 20. + self.v12.hcalThicknessScint + 2*2.,
                                       20. + self.v12.hcalThicknessScint + 2*2., 20. + self.v12.hcalThicknessScint + 2*2.]
        self.v12.hcalNLayers = [100,28,28,26,26]
        self.v12.hcalNStrips = [62,12,12,12,12]
        self.v12.hcalHalfTotalWidthBack = self.v12.hcalNStrips[0]*self.v12.hcalWidthScint/2.
        self.v12.detectors_valid = ["ldmx-det-v12","ldmx-det-v12[.].*","ldmx-det-v9","ldmx-det-v10","ldmx-det-v11"]
        print(self.v12)
