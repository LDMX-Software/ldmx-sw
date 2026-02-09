from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process('target_mumu')
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
from LDMX.Biasing import target
from LDMX.SimCore import generators


p.sequence = [
    target.gamma_mumu(
        'ldmx-det-v14' ,
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.max_events = 1000
p.maxTriesPerEvent = 1000
p.outputFiles = [ 'target_mumu.root' ]
