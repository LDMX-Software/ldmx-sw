from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('builder')
p.run = 1
p.max_events = 10000
p.verbose_parse=True
from LDMX.EventBuilder import eventbuilder
import sys
p.sequence = [
    eventbuilder.from_dat_file(sys.argv[1])
]
p.output_files = [sys.argv[2]]
p.pause()