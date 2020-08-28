from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_en')
from LDMX.Biasing import ecal
p.sequence = [ ecal.electro_nuclear( 'ldmx-det-v12' ) ]
p.maxEvents = 1000
p.termLogLevel = 0
p.logFrequency = 10
p.outputFiles = [ '/tmp/ecal_en.root' ]
