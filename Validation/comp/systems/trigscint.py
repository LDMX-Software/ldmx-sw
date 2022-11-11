features = []

class TrigScint_plots :
    
    def feats() :

        collections=["Sim", "Digi", "Cluster"]
        pads = ["Pad1", "Pad2", "Pad3"] 
        tColl="TrigScintTracks"
        features = [(f'{tColl}/{tColl}_centroid', 'Track centroid [in channel nb]'),
                    (f'{tColl}/{tColl}_n_tracks', 'Track multiplicity'),
                    (f'{tColl}/{tColl}_n_clusters', 'Track cluster multiplicity'),
                    (f'{tColl}/{tColl}_beamEfrac', 'Beam electron energy fraction'),
                    (f'{tColl}/{tColl}_residual', 'Track residual  [in channel nb]'),
                    (f'{tColl}/{tColl}_x', 'Track x [mm]'),
                    (f'{tColl}/{tColl}_y', 'Track y [mm]')
        ]                         
        for pad in pads :
            for coll in collections :
                features +=[
                    (f'TrigScint{coll}{pad}/TrigScint{coll}{pad}_x', f'{coll} x [mm]'),
                    (f'TrigScint{coll}{pad}/TrigScint{coll}{pad}_y', f'{coll} y [mm]'),
                    (f'TrigScint{coll}{pad}/TrigScint{coll}{pad}_z', f'{coll} z [mm]'),
                    (f'TrigScint{coll}{pad}/TrigScint{coll}{pad}_n_hits', 'Hit multiplicity'),
                ]
            features += [(f'TrigScintSim{pad}/TrigScintSim{pad}_hit_time', 'Simhit time [ns]'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_total_pe', 'Total PE in event'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_pe', 'Total PE in bars'),
                         (f'TrigScintSim{pad}/TrigScintSim{pad}_id', 'Channel ID'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_id', 'Channel ID'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_hit_time', 'Digi hit time [ns]'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_id_noise', 'ID of noise hits'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_pe_noise', 'PE in noise hits'),
                         (f'TrigScintDigi{pad}/TrigScintDigi{pad}_n_hits_noise', 'Number of noise hits'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_centroid', 'Cluster centroid [in channel nb]'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_total_pe', 'Cluster total PE in event'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_n_clusters', 'Cluster multiplicity'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_seed', 'Cluster seed [in channel nb]'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_cluster_time', 'Cluster time [ns]'),
                         (f'TrigScintCluster{pad}/TrigScintCluster{pad}_beamEfrac', 'Beam electron energy fraction')
                         #(f'TrigScintDigi{pad}/TrigScintDigi{pad}_beamEfrac', 'Beam electron energy fraction') #not implemented, should be though
            ]

        return features

    feats()

    
