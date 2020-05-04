#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalDigis = ldmxcfg.Producer("ecalDigis","ldmx::EcalDigiProducer")

#energy dep to amplitude multiplier
ecalDigis.parameters[ "gain" ] = 2000.;

#minimum amplitude on empty channels
ecalDigis.parameters[ "pedestal" ] = 1100.;

# padCapacitance to noiseRMS is assumed linear
ecalDigis.parameters[ "noiseIntercept" ] = 700.;
ecalDigis.parameters[ "noiseSlope" ] = 25.;

# capacitance of individual cells
ecalDigis.parameters[ "padCapacitance" ] = 0.1;

# MULTIPLE SAMPLES PER DIGI NOT IMPLEMENTED YET
# Just putting all info into SOI
ecalDigis.parameters[ "nADCs" ] = 10; # number of ADC samples to have per digi
ecalDigis.parameters[ "iSOI"  ] = 0; #index for the sample of interest

# threshold in multiples of noiseRMS as minimum to be readout
ecalDigis.parameters[ "readoutThreshold" ] = 4.;

# should I fill a configuration histogram?
ecalDigis.parameters[ "makeConfigHists" ] = False;
