from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_en')
from LDMX.Biasing import ecal
from LDMX.Ecal import EcalGeometry
p.sequence = [ ecal.electro_nuclear( 'ldmx-det-v12' ) ]
p.maxEvents = 1000
p.outputFiles = [ '/tmp/ecal_en.root' ]
