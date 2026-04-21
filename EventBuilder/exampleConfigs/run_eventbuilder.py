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

# Reduce terminal logging level to see info messages
p.logger = {
    "term_level": 1  # 0=debug, 1=info, 2=warn, 3=error, 4=fatal
}
p.output_files = [sys.argv[2]]
p.pause()
