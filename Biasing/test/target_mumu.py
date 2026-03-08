from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('target_mumu')
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.Biasing import target
from LDMX.SimCore import generators


p.sequence = [
    target.gamma_mumu(
        'ldmx-det-v14' ,
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.max_events = 1000
p.max_tries_per_event = 1000
p.output_files = [ 'target_mumu.root' ]
