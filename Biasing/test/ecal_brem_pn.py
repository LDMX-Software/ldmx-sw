from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_brem_pn')
from LDMX.Biasing import ecal
p.sequence = [ ecal.brem_pn( 'ldmx-det-v12' ) ]
p.maxEvents = 1000
p.termLogLevel = 0
p.logFrequency = 10
p.outputFiles = [ '/tmp/ecal_brem_pn.root' ]
