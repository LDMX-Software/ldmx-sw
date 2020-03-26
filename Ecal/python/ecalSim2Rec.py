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

### v2 
### The default layerWeights and secondOrderEnergyCorrection were calculated at least before v3.
### The other options are below, uncomment those lines if you wish to use those geometries.
#ecalSim2Rec.parameters[ "secondOrderEnergyCorrection" ] = 4000./4220.
##                      ^^^^^^^^^^^^^^^^^^^^^^^^^^^
##   This correction was found by comparing the mean of 1M single 4GeV electron events with 4GeV.
#
#ecalSim2Rec.parameters[ "layerWeights" ] = [
#        1.641, 3.526, 5.184, 6.841,
#        8.222, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775,
#        8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 8.775, 12.642, 16.51,
#        16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 16.51, 8.45
#        ]

### v9
### These layer weights work for all ecal designs between v3 and v9
#ecalSim2Rec.parameters[ "secondOrderEnergyCorrection" ] = 4000./4012.
#                       ^^^^^^^^^^^^^^^^^^^^^^^^^^^
#   This correction was found by comparing the mean of 1M single 4GeV electron events with 4GeV.
#
#ecalSim2Rec.parameters[ "layerWeights" ] = [
#        1.019, 1.707, 3.381, 5.022, 6.679, 8.060, 8.613, 8.613, 8.613, 8.613, 8.613,
#        8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613, 8.613,
#        8.613, 12.480, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347, 16.347,
#        16.347, 8.334
#        ]

## v12
## These layer weights are for the most recent change to the ecal geometry where motherboards and more component space is added
ecalSim2Rec.parameters[ "secondOrderEnergyCorrection" ] = 4000./4010.

ecalSim2Rec.parameters[ "layerWeights" ] = [
        1.675, 2.724, 4.398, 6.039, 7.696, 9.077, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630, 9.630,
        9.630, 13.497, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364, 17.364,
        17.364, 8.990
        ]

