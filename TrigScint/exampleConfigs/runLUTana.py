# Script 3 of 3 to generate new lookup table
# To use the output file, make sure to turn on LUT tracking in your config:
# trig_scint_track.lut_tracking = True
# Creates LUT using PatternLUTMaker for use in LUT-method tracking

from LDMX.Framework import ldmxcfg
from LDMX.Tools.lut_making import PatternLUTMaker

p = ldmxcfg.Process("LUTmaker")

make_lut = PatternLUTMaker()
make_lut.lut_threshold = 1.0 / 1000  # (or whichever threshold you like)
make_lut.input_file = "clusters.txt"  # cluster triplets produced by ClusterTripletMaker
make_lut.output_file = "LUT.txt"

p.sequence = [make_lut]
