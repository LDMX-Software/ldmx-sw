#CONFIG FILE TO VIEW CLUSTERS

from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('clusterviewer')
p.input_files = ["TruthEvents10.root"] #change for new naming conventions
#p.ampl_weighting = False
p.sequence = [ldmxcfg.Analyzer.from_file('ClusterViewerAnalyzer.cxx')]
