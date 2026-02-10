from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('ecal_pn')

import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.Biasing import ecal
from LDMX.SimCore import generators


p.sequence = [
    ecal.photo_nuclear(
        'ldmx-det-v14' ,
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.max_events = 1000
p.max_tries_per_event = 1000
p.output_files = [ 'ecal_pn.root' ]
