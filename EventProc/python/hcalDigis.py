"""Configuration for HcalDigiProducer

Sets all parameters to reasonable defaults.

Examples
--------
>>> from LDMX.EventProc.hcalDigis import hcalDigis
>>> p.sequence.append( hcalDigis )
"""

from LDMX.Framework import ldmxcfg

hcalDigis = ldmxcfg.Producer("hcalDigis", "ldmx::HcalDigiProducer")

hcalDigis.parameters["sim_hit_pass_name"] = ""
hcalDigis.parameters["randomSeed"] = 1
hcalDigis.parameters["meanNoise"] = 0.02
hcalDigis.parameters["readoutThreshold"]= 1
hcalDigis.parameters["strips_side_lr_per_layer"] = 12
hcalDigis.parameters["num_side_lr_hcal_layers"] = 26
hcalDigis.parameters["strips_side_tb_per_layer"] = 12
hcalDigis.parameters["num_side_tb_hcal_layers"] = 28
hcalDigis.parameters["strips_back_per_layer"] = 60 # n strips correspond to 5 cm wide bars
hcalDigis.parameters["num_back_hcal_layers"] = 96
hcalDigis.parameters["super_strip_size"] = 1 # 1 = 5 cm readout, 2 = 10 cm readout, ...
hcalDigis.parameters["mev_per_mip"] = 4.66  # measured 1.4 MeV for a 6mm thick tile, so for 20mm bar = 1.4*20/6
hcalDigis.parameters["pe_per_mip"] = 68. # PEs per MIP at 1m (assume 80% attentuation of 1m)
hcalDigis.parameters["strip_attenuation_length"] = 5. # this is in m
hcalDigis.parameters["strip_position_resolution"] = 150. # this is in mm
