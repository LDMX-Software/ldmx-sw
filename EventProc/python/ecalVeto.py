#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")

ecalVeto.parameters["num_ecal_layers"] = 50
ecalVeto.parameters["back_ecal_starting_layers"] = 22
ecalVeto.parameters["num_layers_for_med_cal"] = 20

