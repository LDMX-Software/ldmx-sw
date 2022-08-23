from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('target_db')
from LDMX.Biasing import target
from LDMX.SimCore import makePath
import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
p.sequence = [
    target.dark_brem( 
        10., #MeV - mass of A'
        makePath.makeLHEPath(10.), #str - full path to directory containing LHE vertex files
        'ldmx-det-v12' , #name of geometry to use
        )
    ]
p.maxEvents = 1000
p.maxTriesPerEvent = 1000
p.outputFiles = [ 'target_db.root' ]
