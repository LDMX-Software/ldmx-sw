#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalDigis = ldmxcfg.Producer("ecalDigis","ldmx::EcalDigiProducer")

# Set the noise (in electrons) when the capacitance is 0.
ecalDigis.parameters["noiseIntercept"] = 900.

# Set the capacitative noise slope (electrons/pF)
ecalDigis.parameters["noiseSlope"] = 22.

# Set the capacitance per cell pad (pF)
ecalDigis.parameters["padCapacitance"] = 27.56

# set the readout threshold in multiples of RMS noise
ecalDigis.parameters["readoutThreshold"] = 4.


### v9
#ecalDigis.parameters[ "secondOrderEnergyCorrection" ] = 0.9976
#   This correction ^^^ was found by comparing the mean of 1M one 4GeV electron events with 4GeV.
#
#ecalDigis.parameters[ "layerWeights" ] = [
#        1.019, 1.707, 3.381, 5.022, 6.679, 8.060, 8.613, 8.613, 8.613, 8.613, 8.613,
#        8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613,
#        8.613, 12.480, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347,
#        16.347, 8.334
#        ]

#### v12
#ecalDigis.parameters[ "secondOrderEnergyCorrection" ] = 1.0
#
#ecalDigis.parameters[ "layerWeights" ] = [
#        1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
#        9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
#        9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
#        17.364, 8.990
#        ]

