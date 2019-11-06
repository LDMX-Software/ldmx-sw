#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalSim2Rec = ldmxcfg.Producer("ecalSim2Rec","ldmx::EcalSim2Rec")

# Set the noise (in electrons) when the capacitance is 0.
ecalSim2Rec.parameters["noiseIntercept"] = 900.

# Set the capacitative noise slope (electrons/pF)
ecalSim2Rec.parameters["noiseSlope"] = 22.

# Set the capacitance per cell pad (pF)
ecalSim2Rec.parameters["padCapacitance"] = 27.56

# set the readout threshold in multiples of RMS noise
ecalSim2Rec.parameters["readoutThreshold"] = 4.
