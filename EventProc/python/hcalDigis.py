#!/usr/bin/python

from LDMX.Framework import ldmxcfg

hcalDigis = ldmxcfg.Producer("hcalDigis", "ldmx::HcalDigiProducer")

hcalDigis.parameters["meanNoise"] = 0.004
hcalDigis.parameters["readoutThreshold"]= 1
hcalDigis.parameters["strips_side_lr_per_layer"] = 3
hcalDigis.parameters["num_side_lr_hcal_layers"] = 17
hcalDigis.parameters["strips_side_tb_per_layer"] = 3
hcalDigis.parameters["num_side_tb_hcal_layers"] = 17
hcalDigis.parameters["strips_back_per_layer"] = 31
hcalDigis.parameters["num_back_hcal_layers"] = 40
hcalDigis.parameters["mev_per_mip"] = 1.4*20./6.
hcalDigis.parameters["pe_per_mip"] = 50.
hcalDigis.parameters["doStrip"] = True

