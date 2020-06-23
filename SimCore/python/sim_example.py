"""A short run-ready example for a basic, unbiased simulation"""

from LDMX.Framework import ldmxcfg
from LDMX.SimCore import examples

p=ldmxcfg.Process("sim")

p.libraries.append("libSimCore.so")
p.libraries.append("libBiasing.so")

single_e = examples.inclusive_single_e()

p.sequence=[single_e]

p.outputFiles=['single_test.root']

p.maxEvents = 10
p.logFrequency = 1
