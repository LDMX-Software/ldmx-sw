#config file to make LUT for tracking

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("LUTmaker")
p.input_files =  ["TruthSamples.root"] #necessary to define but it doesnt use it

make_lut = ldmxcfg.Analyzer.from_file('LUTAnalyzer.cxx', 
                                      needs = ['TrigScint_Event', 'SimCore_Event'])
make_lut.lut_threshold = 1.0/1000
make_lut.input_file = "clusters.txt"
make_lut.output_file = "LUT.txt"

p.sequence = [make_lut]
