#!/usr/bin/python

from LDMX.Framework import ldmxcfg

hcalDigis = ldmxcfg.Producer("hcalDigis", "ldmx::HcalDigiProducer")

# set the mean noise in PE units
hcalDigis.parameters["meanNoise"] = 1.5
hcalDigis.parameters["mev_per_mip"] = 1.4
hcalDigis.parameters["pe_per_mip"] = 13.5
hcalDigis.parameters["doStrip"] = 0

