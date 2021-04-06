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
        self.ThicknessScint = 0.
        self.WidthScint = 0.
        self.ZeroLayer = []
        self.ZeroStrip = []
        self.LayerThickness = []
        self.NumLayers = []
        self.NumStrips = []
        self.HalfTotalWidth = []
        self.EcalDx = 0.
        self.EcalDy = 0.
        self.NumSections = 0
        self.verbose = 0

    def __str__(self) :
        """Stringify this configuration class"""

        s = 'HcalReadoutGeometry { Scintillator thickness: %.1f mm, width: %.1f mm, and half total width (back Hcal): %.1f mm  \n' % ( 
                self.ThicknessScint , self.WidthScint , self.HalfTotalWidthBack )
        s += 'Z positions of zero-th layer %s mm, Z positions of zero-th strip %s mm \n'%(','.join(str(x) for x in self.ZeroLayer),','.join(str(x) for x in self.ZeroStrip))
        s += 'layer thickness %s mm \n'%(','.join(str(x) for x in self.LayerThickness))
        s += 'ecal dx %.1f dy %.1f mm \n'%(self.EcalDx, self.EcalDy)
        s += 'nlayers %s, nstrips %s }'%(' '.join(str(x) for x in self.NumLayers), ' '.join(str(x) for x in self.NumStrips))
        return s

class HcalGeometry() :
    """Container for the various geometries

    """

    def __init__(self):
        self.make_v12()
    
    def make_v12(self) :
        """Create the HcalGeometry with the v12 geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        - ThicknessScint: Scintillator thickness (in z).
          @param gdml see: `scint_thick`.
        
        - WidthScint: Scintillator width (in x).
          Can be obtained by: hcal_length/n_strips.

        - ZeroLayer: Position of the first layer 
          Back Hcal first layer (in z) starts after Ecal+Side-Hcal_z
          Side Hcal first layer (in x or y) starts after Ecal_x/y / 2
          For back Hcal: ecal_front_z + hcal_side_dz (in z)
          For side Hcal: hcal_side_dz/2 (in x/y)
          @param gdml see: `ecal_front_z` and `hcal_side_dz`

        - ZeroStrip: Position of the first strip.
          For back Hcal: back_transverse_width/2 (in x)
          For side Hcal: ecal_front_z (in z)
          @param gdml see: `hcal_back_dx` and `ecal_front_z`

        - LayerThickness: 
          Layer thickness (in z) 
          Can be obtained by: absorber_thickness + scint_thickness + 2.0*air_thickness
          @param gdml see: `side_abso_thick` and `back_abso2_thick` and `air_thick`

        - NumLayers:
          Number of layers per section.

        - NumStrips: 
          Number of strips per layer.

        - NumSections:
          Number of sections. 
          Encoded in HcalID.
          Back, Top, Left, Right, Bottom.

        - EcalDx/EcalDy:
          Length of Ecal needed for side Hcal dimensions.

        - HalfTotalWidth: 
          Half length of a bar
          For back Hcal: nstrips * scint_width / 2 (in x/y)
          For side Hcal: (nlayers_otherside * layer_thick + Ecal_dx(y))/2
          @param gdml see: sideTB_dx, sideTB_dy
        """
        self.v12=HcalReadoutGeometry()

        self.v12.ThicknessScint = 20.0
        self.v12.WidthScint = 50.0
        self.v12.ZeroLayer = [220.+600.,600./2,600./2,600./2,600./2]
        self.v12.ZeroStrip = [3100./2,220.,220.,220.,220.]
        self.v12.LayerThickness = [25. + self.v12.ThicknessScint + 2*2.,
                                   20. + self.v12.ThicknessScint + 2*2., 20. + self.v12.ThicknessScint + 2*2.,
                                   20. + self.v12.ThicknessScint + 2*2., 20. + self.v12.ThicknessScint + 2*2.]
        self.v12.NumSections = 5
        self.v12.NumLayers = [100,28,28,26,26]
        self.v12.NumStrips = [62,12,12,12,12]
        self.v12.EcalDx = 800.0
        self.v12.EcalDy = 600.0
        self.v12.HalfTotalWidth = [(self.v12.NumStrips[0]*self.v12.WidthScint)/2,
                                   (self.v12.NumLayers[3]*self.v12.LayerThickness[3]+self.v12.EcalDx)/2,
                                   (self.v12.NumLayers[4]*self.v12.LayerThickness[4]+self.v12.EcalDx)/2,
                                   (self.v12.NumLayers[1]*self.v12.LayerThickness[1]+self.v12.EcalDy)/2,
                                   (self.v12.NumLayers[2]*self.v12.LayerThickness[2]+self.v12.EcalDy)/2,]
        self.v12.detectors_valid = ["ldmx-det-v12","ldmx-det-v12[.].*","ldmx-det-v9","ldmx-det-v10","ldmx-det-v11"]
