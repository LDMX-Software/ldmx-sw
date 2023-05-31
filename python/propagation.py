from LDMX.Framework.ldmxcfg import Producer



class CustomStatePropagator(Producer):
    """ Producer to generate an user defined set of particles at the target
    and propaget them through the BField map to an user defined plane surface.
    Only forward propagation is supported.

    Parameters
    ----------
    field_map : string
        The field map
    surf_location : double 
        The downstream location of the surface to where to propagate the states
    nstates : int
        Number of states to be propagated
    bs_size : vector<double> [Y,X]
        The size of the generated beamspot in Y-X [mm]
    prange : vector<double> [min,max] 
        Minimum and maximum momentum magnitude uniform pdf  [GeV]
    thetarange : vector<double> [min,max]
        Minimum and maximum theta angle uniform pdf
    phirange : vector<double> [min,max]
        Minimum and maximum phi angle uniform pdf

    """
    
    def __init__(self, instance_name="CustomStatePropagator"):
        super().__init__(instance_name,"tracking::reco::CustomStatePropagator","Tracking")
        self.nstates       = 1000
        self.bs_size       = [40.,10.]
        self.prange        = [0.05,4.]
        self.thetarange    = [0., 1.57079632679]
        self.phirange      = [0., 6.28]
        
