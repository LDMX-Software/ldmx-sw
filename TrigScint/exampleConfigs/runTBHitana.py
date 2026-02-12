import json
from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('plot') #

import sys

input_pass_name="hits"
n_ev=200000

if len(sys.argv) > 2 :
    start_sample=int(sys.argv[2])
else :
    start_sample=2


from LDMX.TrigScint.trigScint import TestBeamHitAnalyzer


# ------------------- all set; setup in detail, and run with these settings ---------------

ts_ev=TestBeamHitAnalyzer("plotMaker")
ts_ev.input_pass_name=input_pass_name
# now in default config, too, but with test beam values :
#these are derived as the mean of gaussian fits to the "event pedestal" (average over middle two quartiles) for each channel
ts_ev.start_sample=start_sample
ts_ev.gain=2e6


p.sequence = [
    ts_ev
    ]


#generate on the fly
p.input_files = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace(".root", "_plots.root")
#p.output_files = [ outname ]

p.histogram_file = outname #.replace(".root"

p.max_events = n_ev

p.logger.file_name=outname.replace(".root",".log")
p.logger.term_level = 2
p.logger.file_level=1#0

p.logger.debug(ts_ev)

json.dumps(p.parameter_dump(), indent=2)
