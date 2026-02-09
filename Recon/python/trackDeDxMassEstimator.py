"""Configuration for TrackDeDxMassEstimator

The tracker dE/dx vs momentum 2d distribution profile 
histogram can be fitted using an approximated 
Bethe-Bloch parametrization at low relativistic regime:
    dE/dx = K * m^2 / p^2 + C,
where m is the particle mass, p is the particle momentum,
K and C are fit parameters.
Using the fitted result of the parameters, the mass of 
the particle can be calculated from its dE/dx and momentum.

Attributes:
------------- 
track_collection : string
    Name of the track collection used as input
fit_res_C : float
    The fitted result of the constant term C (unit: MeV/mm)
fit_res_K : float
    The fitted result of the factor K of the quadratic term (dimentionless)

Examples
--------
    from LDMX.Recon.trackDeDxMassEstimator import recoil_track_mass_estimator
    p.sequence.append( recoil_track_mass_estimator )
"""

from LDMX.Framework import ldmxcfg


class trackDeDxMassEstimator(ldmxcfg.Producer) :
    """Configuration for the mass estimator from tracker dEdx"""

    def __init__(self, name="TrackDeDxMassEstimator") :
        super().__init__(name,'recon::TrackDeDxMassEstimator','Recon')

        self.track_collection = "RecoilTruthTracks"
        self.fit_res_C = 3.094
        self.fit_res_K = 1.862

recoil_track_mass_estimator = trackDeDxMassEstimator("RecoilTrackMassEstimator")
recoil_track_mass_estimator.track_collection = "RecoilTruthTracks"
