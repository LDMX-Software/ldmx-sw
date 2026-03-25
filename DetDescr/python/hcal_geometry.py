"""Configuration for HcalGeometry"""

from LDMX.Framework import field, parameter_set


@parameter_set
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
    back_horizontal_parity:
        Layers with odd parity (1) are horizontal on the x-axis in the back HCal
    y_offset
        Offset of the entire Hcal geometry to be taken into account
    """

    detectors_valid: list[str]
    scint_thickness: float
    scint_width: float
    scint_length: list[float,float]
    zero_layer: list[float]
    zero_strip: list[float,float]
    layer_thickness: list[float]
    num_layers: list[int]
    num_strips: list[int,int]
    half_total_width: list[float,float]
    ecal_dx: float
    ecal_dy: float
    num_sections: int
    verbose: int = 0
    back_horizontal_parity: int = 1
    side_3d_readout: int = 0
    y_offset: float = 0.

    def __str__(self):
        """Stringify this configuration class"""
        s = f"""
        HcalReadoutGeometry {{
            Scintillator thickness: {self.scint_thickness:.1f} [mm],
            width: {self.scint_width:.1f} [mm],
            Number of sections: {{{self.num_sections}}},
            Layers: {{
                Number of layers: {self.num_layers}
                Layer thickness: {self.layer_thickness} [mm]
                Z-position of zero-th layer: {self.zero_layer} [mm]
                Half total width of layers: {self.half_total_width} [mm]
                Number of strips per layer: {self.num_strips}
                Location of zero-th strip per layer: {self.zero_strip} [mm]
                Scintillator length: {self.scint_length} [mm]
            }},
            Ecal DX, DY: {self.ecal_dx}, {self.ecal_dy} [mm],
            Y offset: {self.y_offset},
            Valid detector regexps: {self.detectors_valid}
        }}
        """
        return s

    def make_v13():
        """Create the HcalGeometry with the v13 geometry parameters

        Only sets parameters that must align with the Hcal gdml constants.

        Nothing has changed in v13 for the HCal
        """

        num_sections = 5
        num_layers = [100, 28, 28, 26, 26]
        scint_thickness = 20.0
        scint_width = 50.0

        back_scint_length = 3100.0
        # See https://github.com/LDMX-Software/ldmx-sw/blob/trunk/Detectors/
        #     data/ldmx-det-v13/hcal.gdml#L21
        # and https://github.com/LDMX-Software/ldmx-sw/blob/trunk/Detectors/
        #     data/ldmx-det-v13/hcal.gdml#L177
        # and the corresponding discussion
        #     https://github.com/LDMX-Software/ldmx-sw/pull/1135#discussion_r1178068211
        side_tb_scint_length = 1944.0

        # See https://github.com/LDMX-Software/ldmx-sw/blob/trunk/Detectors/
        #     data/ldmx-det-v13/hcal.gdml#L22
        # and https://github.com/LDMX-Software/ldmx-sw/blob/trunk/Detectors/
        #     data/ldmx-det-v13/hcal.gdml#L181
        # and the corresponding discussion
        #     https://github.com/LDMX-Software/ldmx-sw/pull/1135#discussion_r1178070801
        side_lr_scint_length = 1832.0
        scint_length = [[back_scint_length for layer in range(num_layers[0])],
                        [side_tb_scint_length for layer in range(num_layers[1])],
                        [side_tb_scint_length for layer in range(num_layers[2])],
                        [side_lr_scint_length for layer in range(num_layers[3])],
                        [side_lr_scint_length for layer in range(num_layers[4])],
                        ]

        zero_layer = [
            220.0 + 600.0 + 25.0 + 2 * 2.0,
            600.0 / 2 + 20.0 + 2 * 2.0,
            600.0 / 2 + 20.0 + 2 * 2.0,
            800.0 / 2 + 20.0 + 2 * 2.0,
            800.0 / 2 + 20.0 + 2 * 2.0,
        ]
        zero_strip = [[back_scint_length / 2 for layer in range(num_layers[0])],
                      [220.0 for layer in range(num_layers[1])],
                      [220.0 for layer in range(num_layers[2])],
                      [220.0 for layer in range(num_layers[3])],
                      [220.0 for layer in range(num_layers[4])]]
        layer_thickness = [
            25.0 + scint_thickness + 2 * 2.0,
            20.0 + scint_thickness + 2 * 2.0,
            20.0 + scint_thickness + 2 * 2.0,
            20.0 + scint_thickness + 2 * 2.0,
            20.0 + scint_thickness + 2 * 2.0,
        ]
        num_strips = [[62 for layer in range(num_layers[0])],
                               [12 for layer in range(num_layers[1])],
                               [12 for layer in range(num_layers[2])],
                               [12 for layer in range(num_layers[3])],
                               [12 for layer in range(num_layers[4])]]
        ecal_dx = 800.0
        ecal_dy = 600.0
        half_total_width = [
            [(num_strips[0][layer] * scint_width) / 2
             for layer in range(num_layers[0])],
            [(num_layers[1] * layer_thickness[1] + ecal_dx)
            / 2
             for layer in range(num_layers[1])],
            [(num_layers[2] * layer_thickness[2] + ecal_dx)
            / 2
             for layer in range(num_layers[2])],
            [(num_layers[3] * layer_thickness[3] + ecal_dy)
            / 2
             for layer in range(num_layers[3])],
            [(num_layers[4] * layer_thickness[4] + ecal_dy)
            / 2
             for layer in range(num_layers[4])],
        ]
        return HcalReadoutGeometry(
            num_sections = num_sections,
            num_layers = num_layers,
            scint_thickness = scint_thickness,
            scint_width = scint_width,
            # Layers with odd parity (1) are horizontal (scintillator bar length
            # along the x-axis) in the back hcal
            back_horizontal_parity = 1,
            side_3d_readout = 0,
            # TODO: Check this
            y_offset = 0.,
            scint_length = scint_length,
            zero_layer = zero_layer,
            zero_strip = zero_strip,
            layer_thickness = layer_thickness,
            num_strips = num_strips,
            half_total_width = half_total_width,
            ecal_dx = ecal_dx,
            ecal_dy = ecal_dy,
            detectors_valid = [
                "ldmx-det-v13",
                "ldmx-det-v12",
                "ldmx-det-v12[.].*",
                "ldmx-det-v9",
                "ldmx-det-v10",
                "ldmx-det-v11",
            ]
        )


    def make_v1_prototype():
        """Create the HcalGeometry with the testbeam prototype geometry parameters"""

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
        scint_bar_width = 50.0
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters

        scint_thickness = scint_thickness
        scint_width = scint_bar_width
        scint_length = [[scint_bar_length for layer in range(num_layers)] ]

        # Note that this seems to be location of the first scintillator layer
        zero_layer = [-dz / 2 + air_thickness + absorber_thickness]
        layer_thickness = [layer_thickness]
        num_sections = 1
        num_layers = [num_layers]
        num_strips_front = [num_bars_front for i in range(num_layers_front)]
        num_strips_back = [num_bars_back for i in range(num_layers_back)]
        num_strips_total = num_strips_front + num_strips_back
        num_strips = [num_strips_total]
        # zero_strip and half_total_width are identical
        zero_strip = [[
            N * scint_bar_width / 2 for N in num_strips_total
        ]]
        half_total_width = zero_strip
        return HcalReadoutGeometry(
            num_sections = num_sections,
            num_layers = num_layers,
            scint_thickness = scint_thickness,
            scint_width = scint_width,
            # Layers with odd parity (1) are horizontal (scintillator bar length
            # along the x-axis) in the back hcal
            back_horizontal_parity = 1,
            side_3d_readout = 0,
            # TODO: Check this
            y_offset = 0.,
            scint_length = scint_length,
            zero_layer = zero_layer,
            zero_strip = zero_strip,
            layer_thickness = layer_thickness,
            num_strips = num_strips,
            half_total_width = half_total_width,
            ecal_dx = 0.0,
            ecal_dy = 0.0,
            detectors_valid = [
                "ldmx-hcal-prototype-v1.0",
                "ldmx-hcal-prototype-v1.0[.].*",
            ]
        )


    def make_v2_prototype():
        """Create the HcalGeometry with the testbeam prototype geometry parameters"""
        # GDML-parameters
        absorber_thickness = 20.0
        scint_thickness = 20.0
        scint_bar_length = 2000.0
        scint_bar_cover_thickness = 0.5
        layer_thickness = 45.0
        num_layers_front_vertical = 5
        num_layers_front_horizontal = 4
        num_layers_front = num_layers_front_vertical + num_layers_front_horizontal
        num_layers_back_vertical = 5
        num_layers_back_horizontal = 5
        num_layers_back = num_layers_back_vertical + num_layers_back_horizontal
        num_layers = num_layers_front + num_layers_back
        scint_bar_width = 50.0
        num_bars_front = 8
        num_bars_back = 12
        dz = num_layers * layer_thickness
        # End GDML-parameters

        scint_thickness = scint_thickness
        scint_width = scint_bar_width
        scint_length = [[scint_bar_length for layer in range(num_layers)] ]

        zero_layer = [
            -dz / 2
            + absorber_thickness
            + scint_bar_cover_thickness
            + scint_thickness / 2
        ]
        layer_thickness = [layer_thickness]
        num_sections = 1
        num_layers = [num_layers]
        num_strips_front = [num_bars_front for i in range(num_layers_front)]
        num_strips_back = [num_bars_back for i in range(num_layers_back)]
        num_strips_total = num_strips_front + num_strips_back
        num_strips = [num_strips_total]
        # zero_strip and half_total_width are identical
        zero_strip = [[
            N * scint_bar_width / 2 for N in num_strips_total
        ]]
        half_total_width = zero_strip
        return HcalReadoutGeometry(
            num_sections = num_sections,
            num_layers = num_layers,
            scint_thickness = scint_thickness,
            scint_width = scint_width,
            # Layers with even parity (0) are horizontal (scintillator bar length
            # along the x-axis) in the back HCal
            back_horizontal_parity = 0,
            side_3d_readout = 0,
            # TODO: Check this
            y_offset = 0.,
            scint_length = scint_length,
            zero_layer = zero_layer,
            zero_strip = zero_strip,
            layer_thickness = layer_thickness,
            num_strips = num_strips,
            half_total_width = half_total_width,
            ecal_dx = 0.0,
            ecal_dy = 0.0,
            detectors_valid = [
                "ldmx-hcal-prototype-v2.0",
                "ldmx-hcal-prototype-v2.0[.].*",
            ]
        )

    def make_v14():
        # GDML-parameters
        hcal_air_thick = 2.0
        hcal_scint_thick = 20.0
        hcal_scint_width = 50.0

        back_hcal_num_layers = 96
        back_hcal_num_scint = 40
        back_hcal_abso_thick = 25
        back_hcal_scint_length = 2000.0
        back_hcal_dx = back_hcal_scint_length

        side_hcal_abso_thick = 20.0
        side_hcal_dz = 600.0
        side_hcal_num_modules = 4
        side_hcal_num_sections = 4
        side_hcal_scint_length = [1800.0, 1600.0, 1400.0, 1200.0]
        side_hcal_num_layers = [4, 3, 2, 3]
        side_hcal_num_prev_layers = [0, 4, 7, 9]
        side_hcal_num_scint_z = [m / hcal_scint_width for m in side_hcal_scint_length]
        side_hcal_num_scint_xy = side_hcal_dz / hcal_scint_width
        # Number of layers oriented in x,y. Multiply by 2 to get the total number of
        # layers
        side_hcal_num_total_layers = (
            side_hcal_num_layers[0]
            + side_hcal_num_layers[1]
            + side_hcal_num_layers[2]
            + side_hcal_num_layers[3]
        ) * 2

        ecal_side_dx = 880.6815
        ecal_side_dy = 600.0
        ecal_front_z = 24.0 * 10
        # End GDML-parameters

        scint_thickness = hcal_scint_thick
        scint_width = hcal_scint_width
        ecal_dx = ecal_side_dx
        ecal_dy = ecal_side_dy
        layer_thickness = [
            back_hcal_abso_thick + scint_thickness + 2 * hcal_air_thick,
            side_hcal_abso_thick + scint_thickness + 2 * hcal_air_thick,
            side_hcal_abso_thick + scint_thickness + 2 * hcal_air_thick,
            side_hcal_abso_thick + scint_thickness + 2 * hcal_air_thick,
            side_hcal_abso_thick + scint_thickness + 2 * hcal_air_thick,
        ]
        num_sections = 5
        num_layers = [
            back_hcal_num_layers,
            side_hcal_num_total_layers,
            side_hcal_num_total_layers,
            side_hcal_num_total_layers,
            side_hcal_num_total_layers,
        ]
        # (in absolute numbers)
        zero_layer = [
            ecal_front_z + side_hcal_dz + 2 * hcal_air_thick + back_hcal_abso_thick,
            ecal_side_dy / 2.0 + 2 * hcal_air_thick + side_hcal_abso_thick,
            ecal_side_dy / 2.0 + 2 * hcal_air_thick + side_hcal_abso_thick,
            ecal_side_dx / 2.0 + 2 * hcal_air_thick + side_hcal_abso_thick,
            ecal_side_dx / 2.0 + 2 * hcal_air_thick + side_hcal_abso_thick,
        ]

        # 3D readout for side Hcal
        side_3d_readout = 1
        side_num_modules = side_hcal_num_modules
        # In back hcal: odd layers are horizontal, even layers are vertical
        back_horizontal_parity = 1

        scint_length = [
            [back_hcal_scint_length for layer in range(back_hcal_num_layers)],
            [0.] * side_hcal_num_total_layers,  # Filled below
            [0.] * side_hcal_num_total_layers,
            [0.] * side_hcal_num_total_layers,
            [0.] * side_hcal_num_total_layers,
        ]
        for s in range(side_hcal_num_sections):
            for m in range(side_num_modules):
                for layer_idx in range(side_hcal_num_layers[m] * 2):
                    # Layer numbering starts at 1
                    layer = layer_idx + 1
                    # The back hcal (section 0) has already been handled
                    section_index = s + 1
                    layer_index = layer_idx + side_hcal_num_prev_layers[m] * 2
                    if layer % 2 == 0:
                        # Layer number is even, length is along X/Y
                        scint_length[section_index][layer_index] = (
                            side_hcal_scint_length[m]
                        )
                    else:
                        # Layer number is odd, length is along Z
                        scint_length[section_index][layer_index] = side_hcal_dz
        # side properties
        # num strips
        #  for layer 1: side_hcal_num_scint_z (odd layers have strips oriented in z)
        #  for layer 2: side_hcal_num_scint_xy
        #  [side_hcal_num_scint_z[m],side_hcal_num_scint_xy,etc]
        half_total_width_side = []
        num_strips_side = []
        for m in range(side_num_modules):
            for layer_idx in range(side_hcal_num_layers[m] * 2):
                if (layer_idx + 1) % 2 == 0:
                    # Layer number is even
                    half_total_width_side.append(side_hcal_dz / 2)
                    num_strips_side.append(int(side_hcal_num_scint_xy))
                else:
                    # Layer number is odd
                    half_total_width_side.append(side_hcal_scint_length[m] / 2)
                    num_strips_side.append(int(side_hcal_num_scint_z[m]))

        # In side hcal: odd layers have strips oriented in z
        zero_strip_even = ecal_front_z
        zero_strip_odd = [
            -ecal_side_dx / 2.0,  # Top
            ecal_side_dx / 2.0,   # Bottom
            -ecal_side_dy / 2.0,  # Right
            ecal_side_dy / 2.0,   # Left
        ]
        zero_strip_side = []
        for s in range(side_hcal_num_sections):
            zero_strip_section = []
            for m in range(side_num_modules):
                for layer_idx in range(side_hcal_num_layers[m] * 2):
                    if (layer_idx + 1) % 2 == 0:
                        zero_strip_section.append(zero_strip_even)
                    else:
                        zero_strip_section.append(zero_strip_odd[s])
            zero_strip_side.append(zero_strip_section)

        half_total_width = [
            [back_hcal_dx / 2] * back_hcal_num_layers,
            half_total_width_side,
            half_total_width_side,
            half_total_width_side,
            half_total_width_side,
        ]
        num_strips = [
            [back_hcal_num_scint] * back_hcal_num_layers,
            num_strips_side,
            num_strips_side,
            num_strips_side,
            num_strips_side,
        ]
        zero_strip = [
            # for now x and y is the same so the
            [back_hcal_dx / 2.0] * back_hcal_num_layers,
            zero_strip_side[0],
            zero_strip_side[1],
            zero_strip_side[2],
            zero_strip_side[3],
        ]
        # added the reduced geometry temporarily, for the final geometry
        # we should have a new function "reduced()" with the prototype geom
        detectors_valid = [
            "ldmx-det-v14", "ldmx-det-v14.*", "ldmx-vertTS-v14.*",
            "ldmx-reduced", "ldmx-reduced-v2", "ldmx-reduced-v3",
            "ldmx-lyso-r4-v15", "ldmx-lyso-r4-v15.*",
            "ldmx-det-v15", "ldmx-det-v15.*",
        ]
        return HcalReadoutGeometry(
            num_sections = num_sections,
            num_layers = num_layers,
            scint_thickness = scint_thickness,
            scint_width = scint_width,
            back_horizontal_parity = back_horizontal_parity,
            side_3d_readout = side_3d_readout,
            y_offset = 19.05,
            scint_length = scint_length,
            zero_layer = zero_layer,
            zero_strip = zero_strip,
            layer_thickness = layer_thickness,
            num_strips = num_strips,
            half_total_width = half_total_width,
            ecal_dx = ecal_dx,
            ecal_dy = ecal_dy,
            detectors_valid = detectors_valid
        )


@parameter_set
class HcalGeometry:
    """Container for the various geometries

    Only sets parameters that must align with the Hcal gdml constants.
    """

    v13: HcalReadoutGeometry = field(default_factory = HcalReadoutGeometry.make_v13)
    v14: HcalReadoutGeometry = field(default_factory = HcalReadoutGeometry.make_v14)
    v1_prototype: HcalReadoutGeometry = field(
        default_factory = HcalReadoutGeometry.make_v1_prototype
    )
    v2_prototype: HcalReadoutGeometry = field(
        default_factory = HcalReadoutGeometry.make_v2_prototype
    )
