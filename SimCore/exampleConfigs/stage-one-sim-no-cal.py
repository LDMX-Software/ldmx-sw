from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('simtrk')
p.run = 1
p.max_events = 10
p.output_files = ['stage-one-no-cal.root']

from LDMX.SimCore import simulator, generators, sensitive_detectors
sim = simulator.Simulator('simtrk')
sim.set_detector('ldmx-det-v14-8gev-no-cals', True)
sim.generators = [
    generators.single_8gev_e_upstream_tagger()
]
sim.description = 'single electron 8gev beam, no calorimeters, no biasing or filtering'
sim.sensitive_detectors = [
        sensitive_detectors.TrackerSD.tagger(),
        sensitive_detectors.TrackerSD.recoil(),
        sensitive_detectors.TrigScintSD.target(),
        sensitive_detectors.TrigScintSD.pad1(),
        sensitive_detectors.TrigScintSD.pad2(),
        sensitive_detectors.TrigScintSD.pad3(),
        sensitive_detectors.ScoringPlaneSD.tracker(),
        sensitive_detectors.ScoringPlaneSD.target(),
        sensitive_detectors.ScoringPlaneSD.ecal(),
        sensitive_detectors.ScoringPlaneSD.hcal()
        ]

from LDMX.Tracking.full_tracking_sequence import full_tracking_sequence
trk_seq = full_tracking_sequence(detector="ldmx-det-v14-8gev") 
p.sequence = [
        sim,
        *trk_seq.sequence
]
