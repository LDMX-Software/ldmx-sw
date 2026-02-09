import sys
from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators
from LDMX.SimCore import simulator

this_pass_name="plot"
p = ldmxcfg.Process(this_pass_name)


if len(sys.argv) > 1 :
    in_file=sys.argv[1]
else :
    print("specify an input file")
    exit

p.input_files=[  in_file ]

from LDMX.TrigScint.trigScint import TestBeamClusterAnalyzer

cl_ana_3hit = TestBeamClusterAnalyzer("3-hitClusters")
cl_ana_3hit.inputCollection="TestBeamClustersPad1ThreeHits"

cl_ana_2hit = TestBeamClusterAnalyzer("2-hitClusters")
cl_ana_2hit.inputCollection="TestBeamClustersPad1"



outname=p.input_files[0].replace(".root", "_plots.root")
#p.output_files = [ outname ]
p.histogram_file = outname

p.sequence=[cl_ana_2hit,
            #cl_ana_3hit
            #      ts_ev,
            #      ts_ana
                  ]


p.term_log_level = 2
