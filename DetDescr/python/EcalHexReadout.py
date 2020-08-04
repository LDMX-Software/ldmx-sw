"""Configuration for EcalHexReadout"""

class EcalHexReadout() :
    """Configuration for EcalHexReadout

    Attributes
    ----------
    gap : float
        Distance separating module edges [mm]
    moduleMinR : float
        Module hexagon center-to-flat radius [mm]
    nCellRHeight : float
        Number of cell sides (center-to-corner radii) spanning the module height
    layerZPositions : float
        z-coordinates of sensitive ecal layers relative to front of ecal [mm]
    ecalFrontZ : float
        z-coordinate of front of ecal plane [mm]
    """

    def __init__(self) :
        self.gap = 1.0
        self.moduleMinR = 85.0
        self.nCellRHeight = 35.3
        self.layerZPositions = [
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
                      ]
        self.ecalFrontZ = 240.5
        self.verbose = 0

    def __str__(self) :
        """Stringify this configuration class"""

        s = 'EcalHexReadout { Module Gap: %f mm, Module Radius: %f mm, N Cell Sides Spanning Height: %f }' % ( 
                self.gap , self.moduleMinR , self.nCellRHeight )
        return s

