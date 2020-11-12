from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_brem_pn')
from LDMX.Biasing import ecal
from LDMX.Ecal import EcalGeometry
p.sequence = [ ecal.brem_pn( 'ldmx-det-v12' ) ]
p.maxEvents = 100
p.outputFiles = [ '/tmp/ecal_brem_pn.root' ]
