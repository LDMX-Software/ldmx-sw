#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = 1
<<<<<<< HEAD
ecalVeto.parameters["bdt_file"] = "erin.pkl" 
ecalVeto.parameters["disc_cut"] = 0.94
ecalVeto.parameters["collection_name"] = "EcalVeto"
=======
ecalVeto.parameters["bdt_file"] = "cal_bdt.pkl" 
ecalVeto.parameters["disc_cut"] = 0.999672
>>>>>>> 8b6eac63b072f76349363b0a0ec1b1d9103c12f8
