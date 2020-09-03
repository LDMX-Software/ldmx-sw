from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('ecal_pn')
from LDMX.Biasing import ecal
from LDMX.SimCore import generators
p.sequence = [
    ecal.photo_nuclear( 
        'ldmx-det-v12' , 
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.maxEvents = 1000
p.termLogLevel = 0
p.logFrequency = 10
p.outputFiles = [ '/tmp/ecal_pn.root' ]
