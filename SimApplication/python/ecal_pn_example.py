
from LDMX.Framework import ldmxcfg
from LDMX.Biasing import ecal
from LDMX.SimApplication import generators

p=ldmxcfg.Process("sim")

p.libraries.append("libSimApplication.so")
p.libraries.append("libBiasing.so")

ecal_pn = ecal.photo_nuclear('ldmx-det-v12', generators.single_4gev_e_upstream_tagger())

p.sequence=[ecal_pn]

p.outputFiles=['ecal_pn.root']

p.maxEvents = 1000
p.logFrequency = 100
