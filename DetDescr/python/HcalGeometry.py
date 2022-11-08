"""Configuration for HcalGeometry"""


class HcalReadoutGeometry:
    """Configuration for HcalGeometry for a specific geometry

    Attributes
    ----------
    detectors_valid : array of strings
        Regular expressions identifying which detectors are valid for this geometry
    scint_thickness: Scintillator thickness (in z).
        @param gdml: `scint_thickness`
    scint_width: Scintillator width (in x).
        @param gdml see: `scint_bar_width`
    zero_layer: Position of the first scintillator layer
        Back Hcal first layer (in z) starts after Ecal+Side-Hcal_z
        @param gdml see: `dz`, `air_thickness`, `absorber_thickness`
    zero_strip: Position of the first strip.
        For back Hcal: NumStrips * scint_bar_width / 2 (in x/y)
        @param gdml see: `num_bars_front`, `num_bars_back`, `scint_bar_width`
    half_total_width: Half length of a bar.
        Equal to zero_strip for the prototype geometry.
    layer_thickness:
        Layer thickness (in z)
        Can be obtained by: absorber_thickness + scint_thickness + 2.0*air_thickness
        @param gdml see: `layer_thickness`
    num_layers:
        Number of layers per section.
        @param gdml see: `num_layers`
    num_strips:
        Number of strips per layer.
        @param gdml see: `num_bars_front`, `num_bars_back`, `num_layers_front`,
        and `num_layers_back`
    num_sections:
        Set to 1 for the prototype geometry since the prototype only has the
        Back Hcal.
    ecal_dx/ecal_dy:
        Used in the regular geometry to describe the dimensions of the Ecal.
        Since there is no Ecal in the prototype geometry, these are both set to
        0.
    horizontal_parity:
        Layers with odd parity (1) are horizontal on the x-axis
    """

    def __init__(self):

        # parameters that must align with the geometry
        self.detectors_valid = []
        self.scint_thickness = 0.0
        self.scint_width = 0.0
        self.scint_length = [[]]
        self.zero_layer = []
        self.zero_strip = [[]]
        self.layer_thickness = []
        self.num_layers = []
        self.num_strips = [[]]
        self.half_total_width = [[]]
        self.ecal_dx = 0.0
        self.ecal_dy = 0.0
        self.num_sections = 0
        self.verbose = 0
        self.horizontal_parity = 1
        self.side_3d_readout = 0

    def __str__(self):
        """Stringify this configuration class"""
        s = """
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
                Scintillator length: {} [mm]
            }},
            Ecal DX, DY: {}, {} [mm],
            Valid detector regexps: {}
        }}
        """.format(
            self.scint_thickness,
            self.scint_width,
            self.num_sections,
            self.num_layers,
            self.layer_thickness,
            self.zero_layer,
            self.half_total_width,
            self.num_strips,
            self.zero_strip,
            self.scint_length,
            self.ecal_dx,
            self.ecal_dy,
            self.detectors_valid,
        )
        return s


class HcalGeometry:
    """Container for the various geometries

    Only sets parameters that must align with the Hcal gdml constants.
    """

    def __init__(self):
        self.make_v13()
        self.make_v14()
        self.make_v1_prototype()
        self.make_v2_prototype()

    def make_v13(self):
        """Create the HcalGeometry with the v13 geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        Nothing has changed in v13 for the HCal
        """
        self.v13 = HcalReadoutGeometry()
        self.v13.num_sections = 5
        self.v13.num_layers = [100, 28, 28, 26, 26]

        self.v13.scint_thickness = 20.0
        self.v13.scint_width = 50.0
        self.v13.zero_layer = [
            220.0 + 600.0 + 25.0 + 2 * 2.0,
            600.0 / 2 + 20.0 + 2 * 2.0,
            600.0 / 2 + 20.0 + 2 * 2.0,
            800.0 / 2 + 20.0 + 2 * 2.0,
            800.0 / 2 + 20.0 + 2 * 2.0,
        ]
        self.v13.zero_strip = [[3100.0 / 2 for layer in range(self.v13.num_layers[0])],
                               [220.0 for layer in range(self.v13.num_layers[1])],
                               [220.0 for layer in range(self.v13.num_layers[2])],
                               [220.0 for layer in range(self.v13.num_layers[3])],
                               [220.0 for layer in range(self.v13.num_layers[4])]]
        self.v13.layer_thickness = [
            25.0 + self.v13.scint_thickness + 2 * 2.0,
            20.0 + self.v13.scint_thickness + 2 * 2.0,
            20.0 + self.v13.scint_thickness + 2 * 2.0,
            20.0 + self.v13.scint_thickness + 2 * 2.0,
            20.0 + self.v13.scint_thickness + 2 * 2.0,
        ]
        self.v13.num_strips = [[62 for layer in range(self.v13.num_layers[0])],
                               [12 for layer in range(self.v13.num_layers[1])],
                               [12 for layer in range(self.v13.num_layers[2])],
                               [12 for layer in range(self.v13.num_layers[3])],
                               [12 for layer in range(self.v13.num_layers[4])]]
        self.v13.ecal_dx = 800.0
        self.v13.ecal_dy = 600.0
        self.v13.half_total_width = [
            [(self.v13.num_strips[0][layer] * self.v13.scint_width) / 2 for layer in range(self.v13.num_layers[0])],
            [(self.v13.num_layers[1] * self.v13.layer_thickness[1] + self.v13.ecal_dx)
            / 2
             for layer in range(self.v13.num_layers[1])],
            [(self.v13.num_layers[2] * self.v13.layer_thickness[2] + self.v13.ecal_dx)
            / 2
             for layer in range(self.v13.num_layers[2])],
            [(self.v13.num_layers[3] * self.v13.layer_thickness[3] + self.v13.ecal_dy)
            / 2
             for layer in range(self.v13.num_layers[3])],
            [(self.v13.num_layers[4] * self.v13.layer_thickness[4] + self.v13.ecal_dy)
            / 2
             for layer in range(self.v13.num_layers[4])],
        ]
        self.v13.detectors_valid = [
            "ldmx-det-v13",
            "ldmx-det-v12",
            "ldmx-det-v12[.].*",
            "ldmx-det-v9",
            "ldmx-det-v10",
            "ldmx-det-v11",
        ]
        # Layers with odd parity (1) are horizontal (scintillator bar length
        # along the x-axis)
        self.v13.horizontal_parity = 1
        self.v13.side_3d_readout = 0

    def make_v1_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters"""

        self.v1_prototype = HcalReadoutGeometry()
        # GDML-parameters
        air_thickness = 2.0
        absorber_thickness = 25
        scint_thickness = 20.0
        scint_bar_length = 2000.0
        layer_thickness = absorber_thickness + scint_thickness + 2 * air_thickness
        num_layers_front_vertical = 4
        num_layers_front_horizontal = 5
        num_layers_front = num_layers_front_vertical + num_layers_front_horizontal
        num_layers_back_vertical = 5
        num_layers_back_horizontal = 5
        num_layers_back = num_layers_back_vertical + num_layers_back_horizontal
        num_layers = num_layers_front + num_layers_back
        back_start = num_layers_front * layer_thickness
        scint_bar_width = 50.0
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters

        self.v1_prototype.scint_thickness = scint_thickness
        self.v1_prototype.scint_width = scint_bar_width
        self.v1_prototype.scint_length = [[scint_bar_length for layer in range(num_layers)] ]

        # Note that this seems to be location of the first scintillator layer
        self.v1_prototype.zero_layer = [-dz / 2 + air_thickness + absorber_thickness]
        self.v1_prototype.layer_thickness = [layer_thickness]
        self.v1_prototype.num_sections = 1
        self.v1_prototype.num_layers = [num_layers]
        num_strips_front = [num_bars_front for i in range(num_layers_front)]
        num_strips_back = [num_bars_back for i in range(num_layers_back)]
        num_strips_total = num_strips_front + num_strips_back
        self.v1_prototype.num_strips = [num_strips_total]
        # zero_strip and half_total_width are identical
        self.v1_prototype.zero_strip = [[
            N * scint_bar_width / 2 for N in num_strips_total
        ]]
        self.v1_prototype.half_total_width = self.v1_prototype.zero_strip
        self.v1_prototype.ecal_dx = 0.0
        self.v1_prototype.ecal_dy = 0.0
        self.v1_prototype.detectors_valid = [
            "ldmx-hcal-prototype-v1.0",
            "ldmx-hcal-prototype-v1.0[.].*",
        ]
        # Layers with odd parity (1) are horizontal (scintillator bar length
        # along the x-axis)
        self.v1_prototype.horizontal_parity = 1
        self.v1_prototype.side_3d_readout = 0

    def make_v2_prototype(self):
        """Create the HcalGeometry with the testbeam prototype geometry parameters"""
        self.v2_prototype = HcalReadoutGeometry()
        # GDML-parameters
        absorber_thickness = 20.0
        scint_thickness = 20.0
        scint_bar_length = 2000.0
        scint_bar_cover_thickness = 0.5
        layer_thickness = 45.0
        bar_mounting_plate_thickness = 3.0
        air_thickness = layer_thickness - (
            absorber_thickness
            + bar_mounting_plate_thickness
            + scint_thickness
            + scint_bar_cover_thickness
        )
        num_layers_front_vertical = 5
        num_layers_front_horizontal = 4
        num_layers_front = num_layers_front_vertical + num_layers_front_horizontal
        num_layers_back_vertical = 5
        num_layers_back_horizontal = 5
        num_layers_back = num_layers_back_vertical + num_layers_back_horizontal
        num_layers = num_layers_front + num_layers_back
        back_start = num_layers_front * layer_thickness
        scint_bar_width = 50.0
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters

        self.v2_prototype.scint_thickness = scint_thickness
        self.v2_prototype.scint_width = scint_bar_width
        self.v2_prototype.scint_length = [[scint_bar_length for layer in range(num_layers)] ]

        self.v2_prototype.zero_layer = [
            -dz / 2
            + absorber_thickness
            + scint_bar_cover_thickness
            + scint_thickness / 2
        ]
        self.v2_prototype.layer_thickness = [layer_thickness]
        self.v2_prototype.num_sections = 1
        self.v2_prototype.num_layers = [num_layers]
        num_strips_front = [num_bars_front for i in range(num_layers_front)]
        num_strips_back = [num_bars_back for i in range(num_layers_back)]
        num_strips_total = num_strips_front + num_strips_back
        self.v2_prototype.num_strips = [num_strips_total]
        # zero_strip and half_total_width are identical
        self.v2_prototype.zero_strip = [[
            N * scint_bar_width / 2 for N in num_strips_total
        ]]
        self.v2_prototype.half_total_width = self.v2_prototype.zero_strip
        self.v2_prototype.ecal_dx = 0.0
        self.v2_prototype.ecal_dy = 0.0
        self.v2_prototype.detectors_valid = [
            "ldmx-hcal-prototype-v2.0",
            "ldmx-hcal-prototype-v2.0[.].*",
        ]
        # Layers with even parity (0) are horizontal (scintillator bar length
        # along the x-axis)
        self.v2_prototype.horizontal_parity = 0
        self.v2_prototype.side_3d_readout = 0

    def make_v14(self):
        self.v14 = HcalReadoutGeometry()

        # GDML-parameters
        hcal_airThick = 2.0
        hcal_scintThick = 20.0
        hcal_scintWidth = 50.0

        back_hcal_numLayers = 96
        back_hcal_numScint = 40
        back_hcal_absoThick = 25
        back_hcal_layerThick = (
            back_hcal_absoThick + hcal_scintThick + 2.0 * hcal_airThick
        )
        back_hcal_scint_length = 2000.0
        back_hcal_dx = back_hcal_scint_length
        back_hcal_dy = back_hcal_scint_length
        back_hcal_dz = back_hcal_numLayers * back_hcal_layerThick

        side_hcal_absoThick = 20.0
        side_hcal_dz = 600.0
        side_hcal_numModules = 4
        side_hcal_numSections = 4
        side_hcal_scint_length = [1800.0, 1600.0, 1400.0, 1200.0]
        side_hcal_numLayers = [4, 3, 2, 3]
        side_hcal_numPrevLayers = [0, 4, 7, 9]
        side_hcal_numScintZ = [m / hcal_scintWidth for m in side_hcal_scint_length]
        side_hcal_numScintXY = side_hcal_dz / hcal_scintWidth
        # Number of layers oriented in x,y. Multiply by 2 to get the total number of layers
        side_hcal_numTotalLayers = (
            side_hcal_numLayers[0]
            + side_hcal_numLayers[1]
            + side_hcal_numLayers[2]
            + side_hcal_numLayers[3]
        ) * 2
        side_hcal_layerThick = (
            side_hcal_absoThick + 2.0 * hcal_airThick + hcal_scintThick
        )
        side_hcal_moduleWidth = side_hcal_numTotalLayers * side_hcal_layerThick
        side_hcal_moduleLength = side_hcal_scint_length[0]

        hcal_envelope_dx = 3000.0
        hcal_envelope_dy = 3000.0
        hcal_envelope_dz = back_hcal_dz + side_hcal_dz
        hcal_dz = back_hcal_dz + side_hcal_dz

        ecal_side_dx = 880.6815
        ecal_side_dy = 600.0
        ecal_front_z = 24.0 * 10
        # End GDML-parameters

        self.v14.scint_thickness = hcal_scintThick
        self.v14.scint_width = hcal_scintWidth
        self.v14.ecal_dx = ecal_side_dx
        self.v14.ecal_dy = ecal_side_dy
        self.v14.layer_thickness = [
            back_hcal_absoThick + self.v14.scint_thickness + 2 * hcal_airThick,
            side_hcal_absoThick + self.v14.scint_thickness + 2 * hcal_airThick,
            side_hcal_absoThick + self.v14.scint_thickness + 2 * hcal_airThick,
            side_hcal_absoThick + self.v14.scint_thickness + 2 * hcal_airThick,
            side_hcal_absoThick + self.v14.scint_thickness + 2 * hcal_airThick,
        ]
        self.v14.num_sections = 5
        self.v14.num_layers = [
            back_hcal_numLayers,
            side_hcal_numTotalLayers,
            side_hcal_numTotalLayers,
            side_hcal_numTotalLayers,
            side_hcal_numTotalLayers,
        ]
        # (in absolute numbers)
        self.v14.zero_layer = [
            ecal_front_z + side_hcal_dz + 2 * hcal_airThick + back_hcal_absoThick,
            ecal_side_dy / 2.0 + 2 * hcal_airThick + side_hcal_absoThick,
            ecal_side_dy / 2.0 + 2 * hcal_airThick + side_hcal_absoThick,
            ecal_side_dx / 2.0 + 2 * hcal_airThick + side_hcal_absoThick,
            ecal_side_dx / 2.0 + 2 * hcal_airThick + side_hcal_absoThick,
        ]

        # 3D readout for side Hcal
        self.v14.side_3d_readout = 1
        self.v14.side_num_modules = side_hcal_numModules
        # In back hcal: odd layers are horizontal, even layers are vertical
        self.v14.horizontal_parity = 1
        # In side hcal: odd layers have strips oriented in z
        zero_strip_odd = [
            -ecal_side_dx / 2.0,
            ecal_side_dx / 2.0,
            -ecal_side_dy / 2.0,
            ecal_side_dy / 2.0,
        ]

        self.v14.scint_length = [[back_hcal_scint_length for layer in range(back_hcal_numLayers)],
                                 [0.] * side_hcal_numTotalLayers, # Filled below
                                 [0.] * side_hcal_numTotalLayers,
                                 [0.] * side_hcal_numTotalLayers,
                                 [0.] * side_hcal_numTotalLayers]
        for s in range(side_hcal_numSections):
            for m in range(self.v14.side_num_modules):
                for l in range(side_hcal_numLayers[m] * 2):
                    layer = l + 1
                    section_index = s + 1
                    layer_index = l + side_hcal_numPrevLayers[m] * 2
        # side properties
        # num strips
        #  for layer 1: side_hcal_numScintZ (odd layers have strips oriented in z)
        #  for layer 2: side_hcal_numScintXY
        #  [side_hcal_numScintZ[m],side_hcal_numScintXY,etc]
        half_total_width_side = []
        num_strips_side = []
        for m in range(self.v14.side_num_modules):
            for l in range(side_hcal_numLayers[m] * 2):
                if (l + 1) % 2 == 0:
                    half_total_width_side.append(side_hcal_dz / 2)
                    num_strips_side.append(int(side_hcal_numScintXY))
                else:
                    half_total_width_side.append(side_hcal_length[m] / 2)
                    num_strips_side.append(int(side_hcal_numScintZ[m]))

        zero_strip_side = []
        for s in range(side_hcal_numSections):
            zero_strip_section = []
            for m in range(self.v14.side_num_modules):
                for l in range(side_hcal_numLayers[m] * 2):
                    if (l + 1) % 2 == 0:
                        zero_strip_section.append(ecal_front_z)
                    else:
                        zero_strip_section.append(zero_strip_odd[s])
            zero_strip_side.append(zero_strip_section)

        self.v14.half_total_width = [
            [back_hcal_dx / 2] * back_hcal_numLayers,
            half_total_width_side,
            half_total_width_side,
            half_total_width_side,
            half_total_width_side,
        ]
        self.v14.num_strips = [
            [back_hcal_numScint] * back_hcal_numLayers,
            num_strips_side,
            num_strips_side,
            num_strips_side,
            num_strips_side,
        ]
        self.v14.zero_strip = [
            # for now x and y is the same so the
            [back_hcal_dx / 2.0] * back_hcal_numLayers,
            zero_strip_side[0],
            zero_strip_side[1],
            zero_strip_side[2],
            zero_strip_side[3],
        ]
        self.v14.detectors_valid = ["ldmx-det-v14"]
