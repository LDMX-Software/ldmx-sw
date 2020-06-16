"""Configured HCalDQM python object

Contains an instance of HCalDQM that
has already been configured.

Builds the necessary histograms as well.

Examples
--------
    from LDMX.DQM.hcal import hcal
"""


from LDMX.Framework import ldmxcfg

hcal = ldmxcfg.Analyzer("HCal", "ldmx::HCalDQM")
hcal.parameters["ecal_veto_collection"] = "EcalVeto"

titles = ['', '_track_veto', '_bdt', '_hcal_veto', '_track_bdt', '_vetoes']
for t in titles: 
    hcal.buildHistogram("max_pe%s" % t, "Max Photoelectrons in an HCal Module", 1500, 0, 1500)
    hcal.buildHistogram("total_pe%s" % t, "Total Photoelectrons", 3000, 0, 3000)
    hcal.buildHistogram("n_hits%s" % t, "HCal hit multiplicity", 300, 0, 300)
    hcal.buildHistogram("hit_time_max_pe%s" % t, "Max PE hit time (ns)", 1600, -100, 1500)
    hcal.buildHistogram("min_time_hit_above_thresh%s" % t, "Earliest time of HCal hit above threshold (ns)", 1600, -100, 1500)

hcal.buildHistogram("pe", "Photoelectrons in an HCal Module", 1500, 0, 1500)
hcal.buildHistogram("hit_time", "HCal hit time (ns)", 1600, -100, 1500)
hcal.buildHistogram("veto", "Passes Veto", 4, -1, 3)

