"""Plotting of TrigScint-related validation plots"""

from ._differ import Differ

def plot(hd : Differ, ed : Differ, out_dir = None) :
    """Plot TrigScint-related validation plots

    Parameters
    ----------
    hd : Differ
        Differ containing files that are not event files (presumably histogram files)
    ed : Differ
        Differ containing files that are event files
    """

    collections=["Sim", "Digi", "Cluster"]
    pads = ["Pad1", "Pad2", "Pad3"] 
    tColl="TrigScintTracks"
    features = [
    ]

    for member, name in [
        ('centroid', 'Track centroid [in channel nb]'),
        ('n_tracks', 'Track multiplicity'),
        ('n_clusters', 'Track cluster multiplicity'),
        ('beamEfrac', 'Beam electron energy fraction'),
        ('residual', 'Track residual  [in channel nb]'),
        ('x', 'Track x [mm]'),
        ('y', 'Track y [mm]')
        ] :
        hd.plot1d(f'{tColl}/{tColl}_{member}', name,
                  file_name = re.sub(r'^.*/','',path),
                  out_dir = out_dir)
                  
    for pad in pads :
        for coll in collections :
            for member, name in [ ('x', 'x [mm]'), ('y', 'y [mm]'), ('z', 'z [mm]'), ('n_hits', 'Hit multiplicity') ] :
                hd.plot1d(f'TrigScint{coll}{pad}/TrigScint{coll}{pad}_{member}', f'{coll} {name}',
                          file_name = re.sub(r'^.*/','',path),
                          out_dir = out_dir)
        for member, name in [(f'TrigScintSim{pad}/TrigScintSim{pad}_hit_time', 'Simhit time [ns]'),
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
                             # not implemented but should be
                             #(f'TrigScintDigi{pad}/TrigScintDigi{pad}_beamEfrac', 'Beam electron energy fraction') 
                             ] :
            hd.plot1d(member, name,
                      file_name = re.sub(r'^.*/','',path),
                      out_dir = out_dir)

