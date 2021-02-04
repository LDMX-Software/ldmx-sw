from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_db')
from LDMX.Biasing import eat
from LDMX.SimCore import makePath
from LDMX.Ecal import EcalGeometry
p.sequence = [
    eat.dark_brem( 
        10., #MeV - mass of A'
        makePath.makeLHEPath(10.), #str - full path to directory containing LHE vertex files
        'ldmx-det-v12' , #name of geometry to use
        )
    ]
p.maxEvents = 100
p.outputFiles = [ '/tmp/ecal_db.root' ]
