from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('conv') #

import sys

input_pass_name="unpack"
n_ev=400000

#p.max_events = n_ev

time_offset = int(sys.argv[2]) if len(sys.argv) > 2 else 5
log_verbosity = int(sys.argv[3]) if len(sys.argv) > 3 else 2


from LDMX.TrigScint.trig_scint import EventReadoutProducer


# ------------------- all set; setup in detail, and run with these settings
# ---------------

ts_ev=EventReadoutProducer("eventLinearizer")
ts_ev.input_pass_name=input_pass_name
ts_ev.input_collection="decodedQIEUp"
ts_ev.time_shift=time_offset
if log_verbosity < 1 :
    ts_ev.verbose=True
p.sequence = [
    ts_ev
    ]


#generate on the fly
p.input_files = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace("_reco.root", ".root")
outname=outname.replace(".root", "_linearize.root")
p.output_files = [ outname ]


p.logger.file_path=p.output_files[0].replace(".root",".log")
p.logger.term_level = 2
p.logger.file_level = log_verbosity

#p.pause()

if (log_verbosity < 1 ) :
    p.logger.debug(ts_ev)
