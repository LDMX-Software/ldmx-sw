import sys
from LDMX.Framework import ldmxcfg
p=ldmxcfg.Process("recon")
p.libraries.append("libEventProc.so")
ecalDigis = ldmxcfg.Producer("ecalDigis","ldmx::EcalDigiProducer")
ecalDigis.parameters["meanNoise"] = 0.015
ecalDigis.parameters["readoutThreshold"] = ecalDigis.parameters["meanNoise"]*3
hcalDigis = ldmxcfg.Producer("hcalDigis", "ldmx::HcalDigiProducer")
hcalDigis.parameters["meanNoise"] = 1.5
ecalVeto = ldmxcfg.Producer("ecalVeto", "ldmx::EcalVetoProcessor")
ecalVeto.parameters["num_ecal_layers"] = 50
ecalVeto.parameters["back_ecal_starting_layer"] = 22
ecalVeto.parameters["n_bdt_vars"] = 54
ecalVeto.parameters["disc_cut"] = .5
trigger = ldmxcfg.Producer("trigger", "ldmx::TriggerProcessor")
trigger.parameters["threshold"] = 12.0
trigger.parameters["mode"] = 0
trigger.parameters["start_layer"] = 1
trigger.parameters["end_layer"] = 16
findable_track = ldmxcfg.Producer("findable", "ldmx::FindableTrackProcessor")
p.sequence=[ecalDigis, hcalDigis, ecalVeto, trigger, findable_track]
p.inputFiles=["ldmx_sim_events.root"]
p.outputFiles=["ldmx_sim_recon_events.root"]
p.histogramFile="histo.root"
#p.maxEvents=50
p.printMe()
