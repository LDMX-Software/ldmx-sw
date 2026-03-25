from LDMX.Framework import Processor, processor


@processor("tracking::reco::CustomStatePropagator", "Tracking")
class CustomStatePropagator(Processor):
    """Producer to generate an user defined set of particles at the target
    and propagate them through the BField map to an user defined plane surface.
    Only forward propagation is supported.

    Attributes
    ----------
    nstates : int
        Number of states to be propagated
    bs_size : list[float]
        The size of the generated beamspot in Y-X [mm]
    prange : list[float]
        Minimum and maximum momentum magnitude uniform pdf [GeV]
    thetarange : list[float]
        Minimum and maximum theta angle uniform pdf
    phirange : list[float]
        Minimum and maximum phi angle uniform pdf
    """

    nstates: int = 1000
    bs_size: list[float] = [40.0, 10.0]
    prange: list[float] = [0.05, 4.0]
    thetarange: list[float] = [0.0, 1.57079632679]
    phirange: list[float] = [0.0, 6.28]
