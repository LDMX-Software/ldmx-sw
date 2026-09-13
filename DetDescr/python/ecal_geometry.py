"""Configuration for EcalHexReadout"""

from LDMX.Framework import parameter_set


@parameter_set
class EcalGeometry:
    """Configuration for EcalHexReadout for a specific geometry

    Attributes
    ----------
    layer_z_positions : float
        z-coordinates of sensitive ecal layers relative to front of ecal [mm]
    ecal_front_z : float
        z-coordinate of front of ecal plane [mm]
    gap : float
        Distance separating module edges [mm]
    corners_side_up : bool
        Are the corners of the modules pointed up? (or are the flat sides?)
    layer_shift_x : float
        Shift in x [mm] for the layers that are shifted
    layer_shift_y : float
        Shift in y [mm] for the layers that are shifted
    layer_shift_odd : bool
        Shift the odd-numbered layers
    layer_shift_odd_bilayer : bool
        Shift the odd-numbered bi-layers
    si_thickness : float
        Thickness of the silicon sensor [mm]
    detectors_valid : array of strings
        Regular expressions identifying which detectors are valid for this geometry
    module_min_r : float
        Module hexagon center-to-flat radius [mm]
        UNLIKELY TO CHANGE - will only change if the CMS HGCAL High-Density
        design changes
    n_cell_r_height : float
        Number of cell sides (center-to-corner radii) spanning the module height
        UNLIKELY TO CHANGE - will only change if the CMS HGCAL High-Density
        design changes
    """

    layer_z_positions: list[float]
    ecal_front_z: float
    gap: float
    corners_side_up: bool
    detectors_valid: list[str]
    layer_shift_x: float = 0.0
    layer_shift_y: float = 0.0
    layer_shift_odd: bool = False
    layer_shift_odd_bilayer: bool = False
    si_thickness: float = 0.3
    n_cell_r_height: float = 35.3
    module_min_r: float = 85.0

    def v9():
        return EcalGeometry(
            detectors_valid=["ldmx-det-v9", "ldmx-det-v10", "ldmx-det-v11"],
            gap=0.0,
            layer_z_positions=[
                4.550,
                7.300,
                13.800,
                18.200,
                26.050,
                31.950,
                41.050,
                47.450,
                56.550,
                62.950,
                72.050,
                78.450,
                87.550,
                93.950,
                103.050,
                109.450,
                118.550,
                124.950,
                134.050,
                140.450,
                149.550,
                155.950,
                165.050,
                171.450,
                184.050,
                193.950,
                206.550,
                216.450,
                229.050,
                238.950,
                251.550,
                261.450,
                274.050,
                283.950,
            ],
            ecal_front_z=200.0,
            corners_side_up=False,
        )

    def v12():
        return EcalGeometry(
            detectors_valid=["ldmx-det-v12", "ldmx-det-v12[.].*"],
            gap=1.5,
            layer_z_positions=[
                7.850,
                13.300,
                26.400,
                33.500,
                47.950,
                56.550,
                72.250,
                81.350,
                97.050,
                106.150,
                121.850,
                130.950,
                146.650,
                155.750,
                171.450,
                180.550,
                196.250,
                205.350,
                221.050,
                230.150,
                245.850,
                254.950,
                270.650,
                279.750,
                298.950,
                311.550,
                330.750,
                343.350,
                362.550,
                375.150,
                394.350,
                406.950,
                426.150,
                438.750,
            ],
            ecal_front_z=240.5,
            corners_side_up=False,
        )

    def v13():
        return EcalGeometry(
            detectors_valid=["ldmx-det-v13", "ldmx-det-v13[.].*"],
            gap=1.5,
            layer_z_positions=[
                7.850,
                13.300,
                26.400,
                33.500,
                47.950,
                56.550,
                72.250,
                81.350,
                97.050,
                106.150,
                121.850,
                130.950,
                146.650,
                155.750,
                171.450,
                180.550,
                196.250,
                205.350,
                221.050,
                230.150,
                245.850,
                254.950,
                270.650,
                279.750,
                298.950,
                311.550,
                330.750,
                343.350,
                362.550,
                375.150,
                394.350,
                406.950,
                426.150,
                438.750,
            ],
            ecal_front_z=240.5,
            corners_side_up=True,
        )

    def v14():
        eg = EcalGeometry(
            detectors_valid=["ldmx-det-v14", "ldmx-det-v14.*", "ldmx-vertTS-v14.*"],
            gap=1.5,
            layer_z_positions=[
                7.582,
                16.062,
                33.226,
                43.206,
                60.370,
                71.350,
                90.014,
                101.594,
                120.258,
                131.838,
                150.502,
                162.082,
                180.746,
                192.326,
                210.990,
                222.570,
                241.234,
                252.814,
                271.478,
                283.058,
                301.722,
                313.302,
                331.966,
                343.546,
                365.710,
                380.690,
                402.854,
                417.834,
                439.998,
                454.978,
                477.142,
                492.122,
                514.286,
                529.266,
            ],
            ecal_front_z=240.0,
            corners_side_up=True,
            layer_shift_odd=True,
        )
        # shift by a single cell diameter
        eg.layer_shift_x = 2 * eg.module_min_r / eg.n_cell_r_height
        return eg

    def v15():
        eg = EcalGeometry(
            detectors_valid=[
                "ldmx-det-v15",
                "ldmx-det-v15.*",
                "ldmx-lyso-r4-v15",
                "ldmx-lyso-r4-v15.*",
                "ldmx-ti-v15-8gev",
                "ldmx-ti-v15-8gev.*",
                "ldmx-al-v15-8gev",
                "ldmx-al-v15-8gev.*",
                "ldmx-det-v16-8gev",
            ],
            gap=1.5,
            si_thickness=0.4,
            layer_z_positions=[
                # using 400 um Si thickness
                7.582,
                16.162,
                33.426,
                43.506,
                60.770,
                71.850,
                91.114,
                103.194,
                122.458,
                134.538,
                153.802,
                165.882,
                185.146,
                197.226,
                216.490,
                228.570,
                247.834,
                259.914,
                279.178,
                291.258,
                310.522,
                322.602,
                341.866,
                353.946,
                377.210,
                392.290,
                415.554,
                430.634,
                453.898,
                468.978,
                492.242,
                507.322,
            ],
            ecal_front_z=240.0,
            corners_side_up=True,
            layer_shift_odd=True,
        )
        # shift by a single cell diameter
        eg.layer_shift_x = 2 * eg.module_min_r / eg.n_cell_r_height
        return eg

    def reduced():
        eg = EcalGeometry(
            detectors_valid=["ldmx-reduced"],
            gap=1.5,
            layer_z_positions=[7.448, 16.064, 33.229, 43.202, 60.366, 71.345],
            ecal_front_z=240.0,
            corners_side_up=True,
            layer_shift_odd=True,
        )
        # shift by a single cell diameter
        eg.layer_shift_x = 2 * eg.module_min_r / eg.n_cell_r_height
        return eg

    def reduced_v2():
        eg = EcalGeometry(
            detectors_valid=["ldmx-reduced-v2"],
            gap=1.5,
            layer_z_positions=[9.932, 22.412, 40.81, 54.556, 71.954, 85.67],
            ecal_front_z=240.0,
            corners_side_up=True,
            layer_shift_odd=True,
        )
        # shift by a single cell diameter
        eg.layer_shift_x = 2 * eg.module_min_r / eg.n_cell_r_height
        return eg

    def reduced_v3():
        eg = EcalGeometry(
            detectors_valid=["ldmx-reduced-v3"],
            gap=1.5,
            layer_z_positions=[86.00, 92.23, 136.8, 143.03],
            ecal_front_z=240.0,
            corners_side_up=True,
            layer_shift_odd=True,
        )
        eg.layer_shift_x = 2 * eg.module_min_r / eg.n_cell_r_height
        return eg

    def geometries():
        return [
            EcalGeometry.v9(),
            EcalGeometry.v12(),
            EcalGeometry.v13(),
            EcalGeometry.v14(),
            EcalGeometry.v15(),
            EcalGeometry.reduced(),
            EcalGeometry.reduced_v2(),
            EcalGeometry.reduced_v3(),
        ]
