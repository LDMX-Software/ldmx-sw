from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_pn')

from LDMX.Biasing import ecal
from LDMX.SimCore import generators
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
p.sequence = [
    ecal.photo_nuclear( 
        'ldmx-det-v12' , 
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.maxEvents = 1000
p.maxTriesPerEvent = 1000
p.outputFiles = [ 'ecal_pn.root' ]
