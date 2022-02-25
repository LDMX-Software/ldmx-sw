from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('conv') 

import sys

inputPassName="unpack"
nEv=65000


from LDMX.TrigScint.trigScint import EventReadoutProducer


# ------------------- all set; setup in detail, and run with these settings ---------------

tsEv=EventReadoutProducer("eventLinearizer")
tsEv.input_pass_name=inputPassName
tsEv.input_collection="decodedQIEUp"

p.sequence = [
    tsEv
    ]


#generate on the fly
p.inputFiles = [sys.argv[1]]
outname=sys.argv[1]
outname=outname.replace("_reco.root", ".root")           #remove it if it's there 
outname=outname.replace(".root", "_linearize.root")      #in any case, append this identifier
p.outputFiles = [ outname ]

p.maxEvents = nEv

p.logFileName=p.outputFiles[0].replace(".root",".log")
p.termLogLevel = 2    #warning
p.logFileLevel=1      #info
