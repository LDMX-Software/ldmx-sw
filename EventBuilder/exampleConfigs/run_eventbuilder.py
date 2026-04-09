from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('unpack')
p.run = 1
p.max_events = 100
from LDMX.EventBuilder import eventbuilder
import sys
p.sequence = [
    eventbuilder.from_dat_file(sys.argv[1])
]
p.output_files = [sys.argv[2]]
p.pause()
