from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_db')
from LDMX.Biasing import ecal
from LDMX.SimCore import makePath
p.sequence = [
    ecal.dark_brem( 
        10., #MeV - mass of A'
        makePath.makeLHEPath(10.), #str - full path to directory containing LHE vertex files
        'ldmx-det-v12' , #name of geometry to use
        )
    ]
p.maxEvents = 1000
p.termLogLevel = 0
p.logFrequency = 10
p.outputFiles = [ 'ecal_db.root' ]
