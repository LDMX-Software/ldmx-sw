from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('unpack') 
import sys

p.inputFiles=[sys.argv[1]]
p.outputFiles = [sys.argv[2]]

from LDMX.BeamInstrumentation.FiberTrackerDecoder import FiberTrackerDecoder
from LDMX.BeamInstrumentation.WhiteRabbitDecoder import WhiteRabbitDecoder

decft = FiberTrackerDecoder.decode()
decwr = WhiteRabbitDecoder.decode()

p.sequence = [
        decft,
        decwr
]
