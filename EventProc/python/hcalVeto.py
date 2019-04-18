
from LDMX.Framework import ldmxcfg 

hcalVeto = ldmxcfg.Producer("HcalVeto", "ldmx::HcalVetoProcessor")
hcalVeto.parameters['pe_threshold'] = 3.0
hcalVeto.parameters['max_time'] = 50.0
