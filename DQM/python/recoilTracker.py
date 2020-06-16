"""Configured RecoilTrackerDQM python object

Contains an instance of RecoilTrackerDQM that
has already been configured.

Builds the necessary histograms as well.

Examples
--------
    from LDMX.DQM.recoilTracker import recoilTracker
"""



from LDMX.Framework import ldmxcfg

recoilTracker = ldmxcfg.Analyzer("RecoilTracker", "ldmx::RecoilTrackerDQM")

recoilTracker.buildHistogram("track_count", "Track Multiplicity", 10, 0, 10)
recoilTracker.buildHistogram("loose_track_count", "Track Multiplicity", 10, 0, 10)
recoilTracker.buildHistogram("axial_track_count", "Track Multiplicity", 10, 0, 10)

recoilTracker.buildHistogram("recoil_vx", "Recoil e^{-} Vertex x (mm)", 120, -30, 30) 
recoilTracker.buildHistogram("recoil_vy", "Recoil e^{-} Vertex y (mm)", 200, -100, 100) 
recoilTracker.buildHistogram("recoil_vz", "Recoil e^{-} Vertex z (mm)", 40, -2, 0)

titles = ['', '_track_veto', '_bdt', '_hcal', '_track_bdt', '_vetoes']
for t in titles: 
    recoilTracker.buildHistogram("tp%s" % t,  "Recoil e^{-} Truth p (MeV)", 255, -50, 2500)
    recoilTracker.buildHistogram("tpt%s" % t, "Recoil e^{-} Truth p_{t} (MeV)", 300, -50, 100)
    recoilTracker.buildHistogram("tpx%s" % t, "Recoil e^{-} Truth p_{x} (MeV)", 100, -10, 10)
    recoilTracker.buildHistogram("tpy%s" % t, "Recoil e^{-} Truth p_{y} (MeV)", 100, -10, 10)
    recoilTracker.buildHistogram("tpz%s" % t, "Recoil e^{-} Truth p_{z} (MeV)", 260, -100, 2500)

