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
        self.make_v13()
        self.make_prototype()


    def make_v13(self) :
        """Create the HcalGeometry with the v12 geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        Nothing has changed in v13 for the HCal
        """
        self.v13=HcalReadoutGeometry()

        self.v13.ThicknessScint = 20.0
        self.v13.WidthScint = 50.0
        self.v13.ZeroLayer = [220.+600.,600./2,600./2,600./2,600./2]
        self.v13.ZeroStrip = [3100./2,220.,220.,220.,220.]
        self.v13.LayerThickness = [25. + self.v13.ThicknessScint + 2*2.,
                                   20. + self.v13.ThicknessScint + 2*2., 20. + self.v13.ThicknessScint + 2*2.,
                                   20. + self.v13.ThicknessScint + 2*2., 20. + self.v13.ThicknessScint + 2*2.]
        self.v13.NumSections = 5
        self.v13.NumLayers = [100,28,28,26,26]
        self.v13.NumStrips = [62,12,12,12,12]
        self.v13.EcalDx = 800.0
        self.v13.EcalDy = 600.0
        self.v13.HalfTotalWidth = [(self.v13.NumStrips[0]*self.v13.WidthScint)/2,
                                   (self.v13.NumLayers[3]*self.v13.LayerThickness[3]+self.v13.EcalDx)/2,
                                   (self.v13.NumLayers[4]*self.v13.LayerThickness[4]+self.v13.EcalDx)/2,
                                   (self.v13.NumLayers[1]*self.v13.LayerThickness[1]+self.v13.EcalDy)/2,
                                   (self.v13.NumLayers[2]*self.v13.LayerThickness[2]+self.v13.EcalDy)/2,]
        self.v13.detectors_valid = ["ldmx-det-v13","ldmx-det-v12","ldmx-det-v12[.].*","ldmx-det-v9","ldmx-det-v10","ldmx-det-v11"]



    def make_v1_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        - ThicknessScint: Scintillator thickness (in z).
            @param gdml see: `scint_thickness`.

        - WidthScint: Scintillator width (in x).
            @param gdml see: `scint_bar_width`

        - ZeroLayer: Position of the first scintillator layer
            Back Hcal first layer (in z) starts after Ecal+Side-Hcal_z
            @param gdml see: `dz`, `air_thickness`, `absorber_thickness`

        - ZeroStrip: Position of the first strip.
            For back Hcal: NumStrips * scint_bar_width / 2 (in x/y)
            @param gdml see: `num_bars_front`, `num_bars_back`, `scint_bar_width`

        - HalfTotalWidth: Half length of a bar.
            Equal to ZeroStrip for the prototype geometry.

        - LayerThickness:
            Layer thickness (in z)
            Can be obtained by: absorber_thickness + scint_thickness + 2.0*air_thickness
            @param gdml see: `layer_thickness`

        - NumLayers:
            Number of layers per section.
            @param gdml see: `num_layers`

        - NumStrips:
            Number of strips per layer.
            @param gdml see: `num_bars_front`, `num_bars_back`, `num_layers_front`,
            and `num_layers_back`

        - NumSections:

            Set to 1 for the prototype geometry since the prototype only has the
            Back Hcal.

        - EcalDx/EcalDy:

            Used in the regular geometry to describe the dimensions of the Ecal.
            Since there is no Ecal in the prototype geometry, these are both set to
            0.

        """
        self.prototype=HcalReadoutGeometry()
        # GDML-parameters
        air_thickness = 2.
        absorber_thickness = 25
        scint_thickness = 20.
        scint_bar_length = 2000.
        layer_thickness = absorber_thickness + scint_thickness + 2 * air_thickness
        num_layers_front_vertical = 4
        num_layers_front_horizontal = 5
        num_layers_front=num_layers_front_vertical + num_layers_front_horizontal
        num_layers_back_vertical = 5
        num_layers_back_horizontal = 5
        num_layers_back=num_layers_back_vertical + num_layers_back_horizontal
        num_layers = num_layers_front + num_layers_back
        back_start=num_layers_front * layer_thickness
        scint_bar_width = 50.
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters


        self.prototype.ThicknessScint = scint_thickness
        self.prototype.WidthScint = scint_bar_width

        # Note that this seems to be location of the first scintillator layer
        self.prototype.ZeroLayer = [-dz/2 + air_thickness + absorber_thickness]
        self.prototype.LayerThickness = [layer_thickness]
        self.prototype.NumSections = 1
        self.prototype.NumLayers = [num_layers]
        NumStrips_front = [num_bars_front for i in range(num_layers_front)]
        NumStrips_back = [num_bars_back for i in range(num_layers_back)]
        self.prototype.NumStrips = NumStrips_front + NumStrips_back
        # ZeroStrip and HalfTotalWidth are identical
        self.prototype.ZeroStrip = [N * scint_bar_width / 2 for N in self.prototype.NumStrips]
        self.prototype.HalfTotalWidth = self.prototype.ZeroStrip
        self.prototype.EcalDx = 0.
        self.prototype.EcalDy = 0.
        self.prototype.detectors_valid = ["ldmx-hcal-prototype-v1.0", "ldmx-hcal-prototype-v1.0[.].*"]

    def make_prototype(self):
        print("Warning: Using the generic make_prototype() function produces the v1 geometry of the testbeam prototype. If this is what you actually wanted, use make_v1_prototype() to suppress this message.")
        return make_v1_prototype()

    def make_v2_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        - ThicknessScint: Scintillator thickness (in z).
            @param gdml see: `scint_thickness`.

        - WidthScint: Scintillator width (in x).
            @param gdml see: `scint_bar_width`

        - ZeroLayer: Position of the first scintillator layer
            Back Hcal first layer (in z) starts after Ecal+Side-Hcal_z
            @param gdml see: `dz`, `air_thickness`, `absorber_thickness`

        - ZeroStrip: Position of the first strip.
            For back Hcal: NumStrips * scint_bar_width / 2 (in x/y)
            @param gdml see: `num_bars_front`, `num_bars_back`, `scint_bar_width`

        - HalfTotalWidth: Half length of a bar.
            Equal to ZeroStrip for the prototype geometry.

        - LayerThickness:
            Layer thickness (in z)
            Can be obtained by: absorber_thickness + scint_thickness + 2.0*air_thickness
            @param gdml see: `layer_thickness`

        - NumLayers:
            Number of layers per section.
            @param gdml see: `num_layers`

        - NumStrips:
            Number of strips per layer.
            @param gdml see: `num_bars_front`, `num_bars_back`, `num_layers_front`,
            and `num_layers_back`

        - NumSections:

            Set to 1 for the prototype geometry since the prototype only has the
            Back Hcal.

        - EcalDx/EcalDy:

            Used in the regular geometry to describe the dimensions of the Ecal.
            Since there is no Ecal in the prototype geometry, these are both set to
            0.

        """
        self.prototype=HcalReadoutGeometry()
        # GDML-parameters
        absorber_thickness = 20.
        scint_thickness = 20.
        scint_bar_length = 2000.
        scint_bar_cover_thickness = 0.5
        layer_thickness = 45.
        bar_mounting_plate_thickness = 3.
        air_thickness = layer_thickness - (absorber_thickness +
                                           bar_mounting_plate_thickness +
                                           scint_thickness +
                                           scint_bar_cover_thickness)
        num_layers_front_vertical = 5
        num_layers_front_horizontal = 4
        num_layers_front=num_layers_front_vertical + num_layers_front_horizontal
        num_layers_back_vertical = 5
        num_layers_back_horizontal = 5
        num_layers_back=num_layers_back_vertical + num_layers_back_horizontal
        num_layers = num_layers_front + num_layers_back
        back_start = num_layers_front * layer_thickness
        scint_bar_width = 50.
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters


        self.prototype.ThicknessScint = scint_thickness
        self.prototype.WidthScint = scint_bar_width

        self.prototype.ZeroLayer = [-dz/2 +
                                    absorber_thickness +
                                    scint_bar_cover_thickness +
                                    scint_thickness / 2
                                    ]
        self.prototype.LayerThickness = [layer_thickness]
        self.prototype.NumSections = 1
        self.prototype.NumLayers = [num_layers]
        NumStrips_front = [num_bars_front for i in range(num_layers_front)]
        NumStrips_back = [num_bars_back for i in range(num_layers_back)]
        self.prototype.NumStrips = NumStrips_front + NumStrips_back
        # ZeroStrip and HalfTotalWidth are identical
        self.prototype.ZeroStrip = [N * scint_bar_width / 2 for N in self.prototype.NumStrips]
        self.prototype.HalfTotalWidth = self.prototype.ZeroStrip
        self.prototype.EcalDx = 0.
        self.prototype.EcalDy = 0.
        self.prototype.detectors_valid = ["ldmx-hcal-prototype-v1.0",
                                          "ldmx-hcal-prototype-v1.0[.].*",
                                          "ldmx-hcal-prototype-v2.0",
                                          "ldmx-hcal-prototype-v2.0[.].*"]


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
