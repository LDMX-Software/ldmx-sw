#CONFIG FILE TO VIEW CLUSTERS

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('clusterviewer')
#p.max_events = 10
p.input_files = ["TruthEvents1000.root"] #change for new naming conventions
#p.ampl_weighting = False
p.sequence = [ldmxcfg.Analyzer.from_file('ClusterViewerAnalyzer.cxx', 
              needs = ['TrigScint_Event', 'SimCore_Event'])]
