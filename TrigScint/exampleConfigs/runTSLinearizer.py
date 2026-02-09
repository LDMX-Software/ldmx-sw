from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('conv') #

import sys

input_pass_name="unpack"
n_ev=400000

#p.max_events = n_ev

if len(sys.argv) > 2 :
    time_offset=int(sys.argv[2])
else :
    time_offset=5
if len(sys.argv) > 3 :
    log_verbosity=int(sys.argv[3])
else :
    log_verbosity=2


from LDMX.TrigScint.trigScint import EventReadoutProducer


# ------------------- all set; setup in detail, and run with these settings ---------------

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
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace("_reco.root", ".root")
outname=outname.replace(".root", "_linearize.root")
p.outputFiles = [ outname ]


p.logger.file_path=p.outputFiles[0].replace(".root",".log")
p.logger.termLevel = 2
p.logger.fileLevel = log_verbosity

#p.pause()

if (log_verbosity < 1 ) :
    p.logger.debug(ts_ev)
