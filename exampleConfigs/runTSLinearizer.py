from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('conv') #

import sys

inputPassName="unpack"
nEv=400000

if len(sys.argv) > 2 :
    timeOffset=int(sys.argv[2])
else :
    timeOffset=5
if len(sys.argv) > 3 :
    logVerbosity=int(sys.argv[3])
else :
    logVerbosity=2


from LDMX.TrigScint.trigScint import EventReadoutProducer


# ------------------- all set; setup in detail, and run with these settings ---------------

tsEv=EventReadoutProducer("eventLinearizer")
tsEv.input_pass_name=inputPassName
tsEv.input_collection="decodedQIEUp"
tsEv.time_shift=timeOffset

p.sequence = [
    tsEv
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace("_reco.root", ".root")
outname=outname.replace(".root", "_linearize.root")
p.outputFiles = [ outname ]

p.maxEvents = nEv

p.logFileName=p.outputFiles[0].replace(".root",".log")
p.termLogLevel = 2
p.fileLogLevel = logVerbosity
