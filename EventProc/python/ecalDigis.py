#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalDigis = ldmxcfg.Producer("ecalDigis","EcalDigiProducer")

# set the mean noise in sim-MEV units
ecalDigis.parameters["meanNoise"]=0.015
# set the readout threshold in sim-MEV units
ecalDigis.parameters["readoutThreshold"]=ecalDigis.parameters["meanNoise"]*3

