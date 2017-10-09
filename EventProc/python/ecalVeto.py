#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = 1
ecalVeto.parameters["bdt_file"] = "cal_bdt.pkl" 
ecalVeto.parameters["disc_cut"] = 0.999672
