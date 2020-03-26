
from LDMX.Framework import ldmxcfg

ecalVeto = ldmxcfg.Producer("EcalVeto","ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 34
ecalVeto.parameters["do_bdt"] = True
ecalVeto.parameters["bdt_file"] = "gabrielle.pkl"
ecalVeto.parameters["cellxy_file"] = "cellxy.txt" 
ecalVeto.parameters["disc_cut"] = 0.99
ecalVeto.parameters["collection_name"] = "EcalVeto"
