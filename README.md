Tracking
---------

An `ldmx-sw` submodule focused on the digitization of tracker hits and track finding/fitting using ACTS.

#### Track Reconstruction

The module implements the full tracking chain, from hit smearing and digitization (wip) to the track finding and fitting. The module leverages the ACTS tracking library, which is imported as a submodule. 

#### Tracks EDM

Tracks are stored in the output file. The EDM provides access to the parameters and the covariance matrix at the target and at the ECAL via two track states stored for each track object. Additionally each track has a perigee representation at the target center. 

#### Running reconstruction. 

A jobOption `Tracking/python/reco.py` provides a standard flow for running track reconstruction within ldmx-sw

## Contributors

<a href="https://github.com/LDMX-Software/Tracking/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/Tracking" />
</a>



