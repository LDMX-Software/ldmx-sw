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

        # parameters that must align with the geometry
        self.gap = 0.
        self.moduleMinR = 0.
        self.layerZPositions = [ ]
        self.ecalFrontZ = 0.

        self.v12() #do v12 geometry by default

        #parameters that can be changed while keeping the gdml the same
        self.nCellRHeight = 35.3
        self.verbose = 0

    def __str__(self) :
        """Stringify this configuration class"""

        s = 'EcalHexReadout { Module Gap: %.1f mm, Module Radius: %.1f mm, N Cell Sides Spanning Height: %.1f }' % ( 
                self.gap , self.moduleMinR , self.nCellRHeight )
        return s

    def v12(self) :
        """Set the EcalHexReadout to have the v12 geometry parameters

        Only sets parameters that must align with the gdml
        """

        self.gap = 1.5
        self.moduleMinR = 85.0
        self.layerZPositions = [
                      7.850, 13.300, 26.400, 33.500, 47.950, 56.550, 72.250, 81.350, 97.050, 106.150,
                      121.850, 130.950, 146.650, 155.750, 171.450, 180.550, 196.250, 205.350, 221.050,
                      230.150, 245.850, 254.950, 270.650, 279.750, 298.950, 311.550, 330.750, 343.350,
                      362.550, 375.150, 394.350, 406.950, 426.150, 438.750 
                      ]
        self.ecalFrontZ = 240.5

    def v9(self) :
        """Set the EcalHexReadout to have the v9 geometry parameters

        Only sets parameters that must align with the gdml
        """

        self.gap = 0.0
        self.moduleMinR = 85.0
        self.layerZPositions = [
                      4.550, 7.300, 13.800, 18.200, 26.050, 31.950, 41.050, 47.450, 56.550, 62.950,
                      72.050, 78.450, 87.550, 93.950, 103.050, 109.450, 118.550, 124.950, 134.050,
                      140.450, 149.550, 155.950, 165.050, 171.450, 184.050, 193.950, 206.550, 216.450,
                      229.050, 238.950, 251.550, 261.450, 274.050, 283.950
                      ]
        self.ecalFrontZ = 200.0

