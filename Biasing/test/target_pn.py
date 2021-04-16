from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('target_pn')
from LDMX.Biasing import target
from LDMX.SimCore import generators
import LDMX.Ecal.EcalGeometry
p.sequence = [
    target.photo_nuclear( 
        'ldmx-det-v12' , 
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.maxEvents = 1000
p.outputFiles = [ '/tmp/target_pn.root' ]
