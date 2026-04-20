#config file to make LUT for tracking

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("LUTmaker")
p.input_files =  ["TruthSamples.root"] #necessary to define but it doesnt use it
#p.output_files = ["Tracks.root"]

p.sequence = [ldmxcfg.Analyzer.from_file('LUTAnalyzer.cxx', needs = ['TrigScint_Event', 'SimCore_Event'])
	         ]
