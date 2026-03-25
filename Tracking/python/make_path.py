""" Helpful python configuration functions for getting the path to installed
data files.

This assumes the installation directory can be accessed via the cmake variable
CMAKE_INSTALL_PREFIX.
"""

import os
import sys


def make_field_map_path() -> str:
    """Get the full path to the fieldmap.

    If the fieldmap doesn't exist, exit the app.

    Returns
    -------
    str
        Full path to the installed fieldmap.
    """
    fieldmap_name = 'BmapCorrected3D_13k_unfolded_scaled_1.15384615385.dat'
    path = f'@CMAKE_INSTALL_PREFIX@/data/fieldmap/{fieldmap_name}'
    if not os.path.isfile(path):
        print(f'ERROR: The file {path} does not exist.')
        sys.exit(1)

    return path

def make_detector_path(det_name : str) -> str:
    """Get the full path to the detector description.

    This will generate a path to detector.gdml, the main entry point for the
    detector description, for a given detector name.

    Parameters
    ----------
    det_name : str
        The name of the detector e.g. ldmx-det-v14

    Returns
    -------
    str
        Full path to the detector.gdml of the given detector.

    """
    path = '@CMAKE_INSTALL_PREFIX@/data/detectors/' + det_name + '/detector.gdml'
    if not os.path.isfile(path) :
        print(f'GDML file \'{path}\' does not exist.')
        sys.exit(1)

    return path
