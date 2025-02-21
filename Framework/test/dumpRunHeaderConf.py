import os
import sys
fileIn = sys.argv[1:]
fileName =  " ".join(sys.argv[1:])

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('myAna')
dumpRunHeader = ldmxcfg.RunHeaderAna()

#import LDMX.Ecal.ecal_hardcoded_conditions
#from LDMX.Ecal import EcalGeometry

p.maxEvents = -1
p.run = 2

p.inputFiles  = fileIn
print(p.inputFiles)

p.sequence = [dumpRunHeader]



