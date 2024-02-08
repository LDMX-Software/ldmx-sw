import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

thisPassName="plot"
p = ldmxcfg.Process(thisPassName)


if len(sys.argv) > 1 :
    inFile=sys.argv[1]
else :
    print("specify an input file")
    exit

p.inputFiles=[  inFile ]

from LDMX.TrigScint.trigScint import TestBeamClusterAnalyzer

clAna3hit = TestBeamClusterAnalyzer("3-hitClusters")
clAna3hit.inputCollection="TestBeamClustersPad1ThreeHits"

clAna2hit = TestBeamClusterAnalyzer("2-hitClusters")
clAna2hit.inputCollection="TestBeamClustersPad1"



outname=p.inputFiles[0].replace(".root", "_plots.root")
#p.outputFiles = [ outname ]
p.histogramFile = outname 

p.sequence=[clAna2hit,
            #clAna3hit
            #      tsEv,
            #      tsAna
                  ]


p.termLogLevel = 2
