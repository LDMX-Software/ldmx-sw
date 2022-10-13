"""Configuration for EcalHexReadout"""

class EcalGeometry() :
    """Configuration for EcalHexReadout for a specific geometry

    Attributes
    ----------
    layerZPositions : float
        z-coordinates of sensitive ecal layers relative to front of ecal [mm]
    ecalFrontZ : float
        z-coordinate of front of ecal plane [mm]
    gap : float
        Distance separating module edges [mm]
    cornersSideUp : bool
        Are the corners of the modules pointed up? (or are the flat sides?)
    layer_shift_x : float
        Shift in x [mm] for the layers that are shifted
    layer_shift_y : float
        Shift in y [mm] for the layers that are shifted
    layer_shift_odd : bool
        Shift the odd-numbered layers
    layer_shift_odd_bilayer : bool
        Shift the odd-numbered bi-layers
    detectors_valid : array of strings
        Regular expressions identifying which detectors are valid for this geometry
    moduleMinR : float
        Module hexagon center-to-flat radius [mm]
        UNLIKELY TO CHANGE - will only change if the CMS HGCAL High-Density design changes
    nCellRHeight : float
        Number of cell sides (center-to-corner radii) spanning the module height
        UNLIKELY TO CHANGE - will only change if the CMS HGCAL High-Density design changes
    """

    def __init__(self,
            layerZPositions,
            ecalFrontZ,
            gap,
            cornersSideUp,
            detectors_valid,
            layer_shift_x = 0.,
            layer_shift_y = 0.,
            layer_shift_odd = False,
            layer_shift_odd_bilayer = False,
            nCellRHeight = 35.3,
            moduleMinR = 85.0) :

        # parameters that must align with the geometry
        self.layerZPositions = layerZPositions
        self.ecalFrontZ = ecalFrontZ
        self.gap = gap
        self.cornersSideUp = cornersSideUp
        self.layer_shift_x = layer_shift_x
        self.layer_shift_y = layer_shift_y
        self.layer_shift_odd = layer_shift_odd
        self.layer_shift_odd_bilayer = layer_shift_odd_bilayer
        self.moduleMinR = moduleMinR
        self.detectors_valid = detectors_valid

        # parameters which are somewhat independent of GDML
        self.nCellRHeight = nCellRHeight

        self.verbose = 0

    def __str__(self) :
        """Stringify this configuration class"""

        s = 'EcalGeometry { Module Gap: %.1f mm, Module Radius: %.1f mm, N Cell Sides Spanning Height: %.1f }' % ( 
                self.gap , self.moduleMinR , self.nCellRHeight )
        return s

    def v9() : 
        return EcalGeometry(detectors_valid = ["ldmx-det-v9","ldmx-det-v10","ldmx-det-v11"],
                gap = 0.0,
                layerZPositions = [
                      4.550, 7.300, 13.800, 18.200, 26.050, 31.950, 41.050, 47.450, 56.550, 62.950,
                      72.050, 78.450, 87.550, 93.950, 103.050, 109.450, 118.550, 124.950, 134.050,
                      140.450, 149.550, 155.950, 165.050, 171.450, 184.050, 193.950, 206.550, 216.450,
                      229.050, 238.950, 251.550, 261.450, 274.050, 283.950
                      ],
                ecalFrontZ = 200.0,
                cornersSideUp = False,
                )
    
    def v12() :
        return EcalGeometry(detectors_valid = ["ldmx-det-v12","ldmx-det-v12[.].*"],
                gap = 1.5,
                layerZPositions = [
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
                      ],
                ecalFrontZ = 240.5,
                cornersSideUp = False,
                )

    def v13() :
        return EcalGeometry(detectors_valid = ["ldmx-det-v13","ldmx-det-v13[.].*"],
                gap = 1.5,
                layerZPositions = [
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
                      ],
                ecalFrontZ = 240.5,
                cornersSideUp = True,
                )

    def v14() :
        eg = EcalGeometry(detectors_valid = ["ldmx-det-v14","ldmx-det-v14[.].*"],
                gap = 1.5,
                layerZPositions = [ 
                      7.932, 14.532, 32.146, 40.746, 58.110, 67.710, 86.574, 96.774, 115.638, 125.838, 
                      144.702, 154.902, 173.766, 183.966, 202.830, 213.030, 231.894, 242.094, 260.958, 
                      271.158, 290.022, 300.222, 319.086, 329.286, 351.650, 365.250, 387.614, 401.214, 
                      423.578, 437.178, 459.542, 473.142, 495.506, 509.106
                      ],
                ecalFrontZ = 240.0,
                cornersSideUp = True,
                layer_shift_odd = True,
                )
        # shift by a single cell diameter
        eg.layer_shift_x = 2*eg.moduleMinR / eg.nCellRHeight
        return eg

    def geometries() :
        return [EcalGeometry.v9(), EcalGeometry.v12(), EcalGeometry.v13(), EcalGeometry.v14()]
