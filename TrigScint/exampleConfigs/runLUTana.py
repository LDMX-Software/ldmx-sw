#Creates LUT using PatternLUTMaker for use in LUT-method tracking

from LDMX.Framework import ldmxcfg
from LDMX.Tools.lut_making import PatternLUTMaker

p = ldmxcfg.Process("LUTmaker")
 
make_lut = PatternLUTMaker()
make_lut.lut_threshold = 1.0/1000 #(or whichever threshold you like)
make_lut.input_file = "clusters.txt" #cluster triplets as produced by ClusterTripletMaker
make_lut.output_file = "LUT.txt"

p.sequence = [make_lut]