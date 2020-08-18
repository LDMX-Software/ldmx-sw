from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('target_en')
from LDMX.Biasing import target
from LDMX.SimCore import generators
p.sequence = [
    target.electro_nuclear( 
        'ldmx-det-v12' , 
        generators.single_4gev_e_upstream_tagger()
        )
    ]
p.maxEvents = 1000
p.termLogLevel = 0
p.logFrequency = 10
p.outputFiles = [ 'target_en.root' ]
