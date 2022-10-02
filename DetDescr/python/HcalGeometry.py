"""Configuration for HcalGeometry"""

class HcalReadoutGeometry() :
    """Configuration for HcalGeometry for a specific geometry

    Attributes
    ----------
    detectors_valid : array of strings
        Regular expressions identifying which detectors are valid for this geometry
    ThicknessScint: Scintillator thickness (in z).
        @param gdml: `scint_thickness`
    WidthScint: Scintillator width (in x).                                                                                                          
        @param gdml see: `scint_bar_width`
    ZeroLayer: Position of the first scintillator layer
        Back Hcal first layer (in z) starts after Ecal+Side-Hcal_z
        @param gdml see: `dz`, `air_thickness`, `absorber_thickness`
    ZeroStrip: Position of the first strip.
        For back Hcal: NumStrips * scint_bar_width / 2 (in x/y)
        @param gdml see: `num_bars_front`, `num_bars_back`, `scint_bar_width`
    HalfTotalWidth: Half length of a bar.
        Equal to ZeroStrip for the prototype geometry.
    LayerThickness:
        Layer thickness (in z)
        Can be obtained by: absorber_thickness + scint_thickness + 2.0*air_thickness
        @param gdml see: `layer_thickness`
    NumLayers:
        Number of layers per section.
        @param gdml see: `num_layers`
    NumStrips:
        Number of strips per layer.
        @param gdml see: `num_bars_front`, `num_bars_back`, `num_layers_front`,
        and `num_layers_back`
    NumSections:
        Set to 1 for the prototype geometry since the prototype only has the
        Back Hcal.
    EcalDx/EcalDy:
        Used in the regular geometry to describe the dimensions of the Ecal.
        Since there is no Ecal in the prototype geometry, these are both set to
        0.
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
        s = '''
        HcalReadoutGeometry {{
            Scintillator thickness: {:.1f} [mm], width: {:.1f} [mm],
            Number of sections: {{{}}},
            Layers: {{
                Number of layers: {}
                Layer thickness: {} [mm]
                Z-position of zero-th layer: {} [mm]
                Half total width of layers: {} [mm]
                Number of strips per layer: {}
                Location of zero-th strip per layer: {} [mm]
            }},
            Ecal DX, DY: {}, {} [mm],
            Valid detector regexps: {}
        }}
        '''.format(self.ThicknessScint, self.WidthScint,
                self.NumSections,
                self.NumLayers, self.LayerThickness, self.ZeroLayer,
                self.HalfTotalWidth,self.NumStrips, self.ZeroStrip,
                self.EcalDx, self.EcalDy,
                self.detectors_valid)
        return s


class HcalGeometry() :
    """Container for the various geometries
    
      Only sets parameters that must align with the Hcal gdml constants.
    """

    def __init__(self):
        self.make_v13()
        self.make_v14()
        self.make_v1_prototype()
        self.make_v2_prototype()

    def make_v13(self) :
        """Create the HcalGeometry with the v13 geometry parameters

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
        # Layers with odd parity (1) are horizontal (scintillator bar length
        # along the x-axis)
        self.v13.horizontal_parity = 1

    def make_v1_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters
        """

        self.v1_prototype=HcalReadoutGeometry()
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


        self.v1_prototype.ThicknessScint = scint_thickness
        self.v1_prototype.WidthScint = scint_bar_width

        # Note that this seems to be location of the first scintillator layer
        self.v1_prototype.ZeroLayer = [-dz/2 + air_thickness + absorber_thickness]
        self.v1_prototype.LayerThickness = [layer_thickness]
        self.v1_prototype.NumSections = 1
        self.v1_prototype.NumLayers = [num_layers]
        NumStrips_front = [num_bars_front for i in range(num_layers_front)]
        NumStrips_back = [num_bars_back for i in range(num_layers_back)]
        self.v1_prototype.NumStrips = NumStrips_front + NumStrips_back
        # ZeroStrip and HalfTotalWidth are identical
        self.v1_prototype.ZeroStrip = [N * scint_bar_width / 2 for N in self.v1_prototype.NumStrips]
        self.v1_prototype.HalfTotalWidth = self.v1_prototype.ZeroStrip
        self.v1_prototype.EcalDx = 0.
        self.v1_prototype.EcalDy = 0.
        self.v1_prototype.detectors_valid = ["ldmx-hcal-prototype-v1.0", "ldmx-hcal-prototype-v1.0[.].*"]
        # Layers with odd parity (1) are horizontal (scintillator bar length
        # along the x-axis)
        self.v1_prototype.horizontal_parity = 1


    def make_v2_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters
        """
        self.v2_prototype=HcalReadoutGeometry()
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

        self.v2_prototype.ThicknessScint = scint_thickness
        self.v2_prototype.WidthScint = scint_bar_width

        self.v2_prototype.ZeroLayer = [-dz/2 +
                                    absorber_thickness +
                                    scint_bar_cover_thickness +
                                    scint_thickness / 2
                                    ]
        self.v2_prototype.LayerThickness = [layer_thickness]
        self.v2_prototype.NumSections = 1
        self.v2_prototype.NumLayers = [num_layers]
        NumStrips_front = [num_bars_front for i in range(num_layers_front)]
        NumStrips_back = [num_bars_back for i in range(num_layers_back)]
        self.v2_prototype.NumStrips = NumStrips_front + NumStrips_back
        # ZeroStrip and HalfTotalWidth are identical
        self.v2_prototype.ZeroStrip = [N * scint_bar_width / 2 for N in self.v2_prototype.NumStrips]
        self.v2_prototype.HalfTotalWidth = self.v2_prototype.ZeroStrip
        self.v2_prototype.EcalDx = 0.
        self.v2_prototype.EcalDy = 0.
        self.v2_prototype.detectors_valid = ["ldmx-hcal-prototype-v2.0",
                                          "ldmx-hcal-prototype-v2.0[.].*"]
        # Layers with even parity (0) are horizontal (scintillator bar length
        # along the x-axis)
        self.v2_prototype.horizontal_parity = 0

    def make_v14(self) :
        self.v14 = HcalReadoutGeometry()
        
        # GDML-parameters
        hcal_airThick = 2.
        hcal_scintThick = 20.
        hcal_scintWidth = 50.
        
        back_hcal_numLayers = 96
        back_hcal_numScint = 40
        back_hcal_absoThick = 25
        back_hcal_layerThick = back_hcal_absoThick + hcal_scintThick + 2.0*hcal_airThick
        back_hcal_dx = 2000.
        back_hcal_dy = 2000.
        back_hcal_dz = back_hcal_numLayers*back_hcal_layerThick

        side_hcal_numSections = 4
        side_hcal_absoThick = 20.
        side_hcal_dz = 600.
        side_hcal_length = [1800.,1600.,1400.,1200.]
        side_hcal_numLayers = [4 3 2 3]
        side_hcal_numScintZ = [l/hcal_scintWidth for l in side_hcal_length]
        side_hcal_numScintXY = side_hcal_dz/hcal_scintWidth
        side_hcal_numTotalLayers = (side_hcal_numLayers[0]+side_hcal_numLayers[1]+side_hcal_numLayers[2]+side_hcal_numLayers[3])*2
        side_hcal_layerThick = side_hcal_absoThick + 2.*hcal_airThick + hcal_scintThick
        side_hcal_moduleWidth = side_hcal_numLayers*side_hcal_layerThick
        side_hcal_moduleLength = side_hcal_length[0]

        hcal_envelope_dx = 3000.
        hcal_envelope_dy = 3000.
        hcal_envelope_dz = back_hcal_dz + side_hcal_dz
        hcal_dz = hcal_back_dz + hcal_side_dz

        ecal_side_dx = 880.6815
        ecal_side_dy = 600.
        ecal_front_z = 24.0*10
        hcal_dz = hcal_back_dz + hcal_side_dz
        
        self.v14.ThicknessScint = hcal_scintThick
        self.v14.WidthScint = hcal_scintWidth
        self.v14.ZeroLayer = [
            ecal_front_z + side_hcal_dz,
            hcal_side_dz/2,
            hcal_side_dz/2,
            hcal_side_dz/2,
            hcal_side_dz/2
        ]
        self.v14.ZeroStrip = [
            hcal_back_dy/2,
            ecal_front_z,
            ecal_front_z,
            ecal_front_z,
            ecal_front_z
        ]
        self.v14.LayerThickness = [
            back_hcal_absoThick + self.v14.ThicknessScint + 2*hcal_airThick,
            back_hcal_absoThick + self.v14.ThicknessScint + 2*hcal_airThick,
            back_hcal_absoThick + self.v14.ThicknessScint + 2*hcal_airThick,
            back_hcal_absoThick + self.v14.ThicknessScint + 2*hcal_airThick,
            back_hcal_absoThick + self.v14.ThicknessScint + 2*hcal_airThick
        ]
        self.v14.NumSections = 5
        self.v14.NumLayers = [
            back_hcal_numLayers,
            28,28,26,26
        ]
        self.v14.NumStrips = [
            back_hcal_numScint,
            12,12,12,12
        ]
        self.v14.EcalDx = side_Ecal_dx
        self.v14.EcalDy = side_Ecal_dy
        self.v14.HalfTotalWidth = [(self.v14.NumStrips[0]*self.v14.WidthScint)/2,
                                   (self.v14.NumLayers[3]*self.v14.LayerThickness[3]+self.v14.EcalDx)/2,
                                   (self.v14.NumLayers[4]*self.v14.LayerThickness[4]+self.v14.EcalDx)/2,
                                   (self.v14.NumLayers[1]*self.v14.LayerThickness[1]+self.v14.EcalDy)/2,
                                   (self.v14.NumLayers[2]*self.v14.LayerThickness[2]+self.v14.EcalDy)/2,]
        self.v14.detectors_valid = ["ldmx-det-v14"]
        
        # Layers with odd parity (1) are horizontal on the x-axis
        self.v14.horizontal_parity = 1

