
from LDMX.Framework import ldmxcfg
from LDMX.SimApplication import examples

p=ldmxcfg.Process("sim")

p.libraries.append("libSimApplication.so")
p.libraries.append("libBiasing.so")

single_e = examples.inclusive_single_e()

p.sequence=[single_e]

p.outputFiles=['single_test.root']

p.maxEvents = 10
p.logFrequency = 1
