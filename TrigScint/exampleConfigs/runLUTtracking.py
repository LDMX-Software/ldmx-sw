# Performs tracking with TrigScintTrackProducer, with+without LUT method for comparison
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process("tracking")
p.input_files = ["Clusters.root"]
p.output_files = ["Tracks.root"]

from LDMX.TrigScint.trig_scint import TrigScintTrackProducer

tstp_tracks = TrigScintTrackProducer()
lut_tracks = TrigScintTrackProducer()

tstp_tracks.delta_max = 1.0  # set to 1.0 for easy comparison to lut_tracks
tstp_tracks.output_collection = "maxdeltaTracks"

lut_tracks.delta_max = 1.0
lut_tracks.lut_tracking = True
lut_tracks.output_collection = "lutTracks"

p.sequence = [tstp_tracks, lut_tracks]
