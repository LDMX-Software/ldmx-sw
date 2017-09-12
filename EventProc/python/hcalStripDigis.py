#!/usr/bin/python

from LDMX.Framework import ldmxcfg

hcalStripDigis = ldmxcfg.Producer("hcalStripDigis", "ldmx::HcalStripDigiProducer")

# set the mean noise in PE units
hcalStripDigis.parameters["meanNoise"] = 2.0

