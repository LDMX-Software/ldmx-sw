#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")

ecalVeto.parameters["num_ecal_layers"] = 33
ecalVeto.parameters["back_ecal_starting_layers"] = 20 
ecalVeto.parameters["num_layers_for_med_cal"] = 10
ecalVeto.parameters["total_dep_cut"] = 25.
ecalVeto.parameters["total_iso_cut"] = 15.
ecalVeto.parameters["back_ecal_cut"] = 1.
ecalVeto.parameters["ratio_cut"] = 10.

