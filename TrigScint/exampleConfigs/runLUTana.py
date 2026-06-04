#config file to make LUT for tracking using PatternLUTMaker

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("LUTmaker")
p.input_files =  ["TruthSamples.root"] #necessary to define but it doesnt use it, the real input file is defined below

from LDMX.TrigScint.trig_scint import PatternLUTMaker

make_lut = PatternLUTMaker("LUTmaker")
make_lut.lut_threshold = 1.0/1000 #or whichever threshold you like
make_lut.input_file = "clusters.txt" #here! input list of cluster triplets
make_lut.output_file = "LUT.txt"

p.sequence = [make_lut]
