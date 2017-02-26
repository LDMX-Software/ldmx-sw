#!/usr/bin/python

from LDMX.Framework import ldmxcfg

hcalDigis = ldmxcfg.Producer("HcalHits", "ldmx::HcalDigiProducer")

# set the mean noise in PE units
hcalDigi.parameters["meanNoise"]=2.0
hcalDigi.parameters["mev_per_mip"]=1.40
hcalDigi.parameters["pe_per_mip"]=13.5*6./4.
hcalDigi.parameters["num_back_hcal_layers"]=50
hcalDigi.parameters["num_wrap_hcal_layers"]=21
hcalDigi.parameters["verbose"]=0
