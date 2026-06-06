#Performs tracking with TrigScintTrackProducer, with and without LUT method for comparison
from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('tracking')
p.input_files = ["Clusters.root"]
p.output_files = ["Tracks.root"]

from LDMX.TrigScint.trig_scint import TrigScintTrackProducer

tstp_tracks = TrigScintTrackProducer("tstp")
lut_tracks = TrigScintTrackProducer("lut")

tstp_tracks.delta_max = 1.0
tstp_tracks.verbosity = 1
tstp_tracks.output_collection = "tstpTracks"

lut_tracks.delta_max = 1.0
lut_tracks.verbosity = 1
lut_tracks.lut_tracking = True
lut_tracks.lut_file = "LUT.txt"
lut_tracks.output_collection = "lutTracks"

p.logger.term_level = 1

p.sequence = [tstp_tracks, 
              lut_tracks
              ]
