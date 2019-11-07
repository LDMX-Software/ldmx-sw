#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalRecon = ldmxcfg.Producer("ecalRecon","ldmx::EcalRecProducer")

ecalRecon.parameters[ "digiCollName" ] = "EcalDigis"
ecalRecon.parameters[ "digiPassName" ] = ""

#ecalRecon.parameters[ "layerWeights" ] # default defined in source file
#ecalRecon.parameters[ "secondOrderEnergyCorrection" ] #default defined in source file
