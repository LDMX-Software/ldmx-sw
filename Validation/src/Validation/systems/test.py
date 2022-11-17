features = []

class Test_plots :
    
    def feats() :
        features = [('TrigScintDigiPad1/TrigScintDigiPad1_n_hits', 'Hit multiplicity'),
                    ('TrigScintTracks/TrigScintTracks_n_tracks', 'Track multiplicity'),
                    ('EcalShowerFeatures/EcalShowerFeatures_num_readout_hits', 'N Readout Hits'),
                    ]

        return features

