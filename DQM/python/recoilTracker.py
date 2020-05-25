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

recoilTracker.build1DHistogram("track_count", "Track Multiplicity", 10, 0, 10)
recoilTracker.build1DHistogram("loose_track_count", "Track Multiplicity", 10, 0, 10)
recoilTracker.build1DHistogram("axial_track_count", "Track Multiplicity", 10, 0, 10)

recoilTracker.build1DHistogram("recoil_vx", "Recoil e^{-} Vertex x (mm)", 120, -30, 30) 
recoilTracker.build1DHistogram("recoil_vy", "Recoil e^{-} Vertex y (mm)", 200, -100, 100) 
recoilTracker.build1DHistogram("recoil_vz", "Recoil e^{-} Vertex z (mm)", 40, -2, 0)

titles = ['', '_track_veto', '_bdt', '_hcal', '_track_bdt', '_vetoes']
for t in titles: 
    recoilTracker.build1DHistogram("tp%s" % t,  "Recoil e^{-} Truth p (MeV)", 255, -50, 2500)
    recoilTracker.build1DHistogram("tpt%s" % t, "Recoil e^{-} Truth p_{t} (MeV)", 300, -50, 100)
    recoilTracker.build1DHistogram("tpx%s" % t, "Recoil e^{-} Truth p_{x} (MeV)", 100, -10, 10)
    recoilTracker.build1DHistogram("tpy%s" % t, "Recoil e^{-} Truth p_{y} (MeV)", 100, -10, 10)
    recoilTracker.build1DHistogram("tpz%s" % t, "Recoil e^{-} Truth p_{z} (MeV)", 260, -100, 2500)

