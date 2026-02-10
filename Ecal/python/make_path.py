"""Helpful python configuration functions for getting the path to installed data files. 

This file was configured by cmake for the installation of ldmx-sw at
   @CMAKE_INSTALL_PREFIX@
"""

import os
import sys


def makeBDTPath( bdt_name ) :
    """Get the full path to the installed BDT files

    Exits entire python script if file does not exist.

    Parameters
    ----------
    bdt_name : str
        Name of BDT file to make path for (no extension)

    Returns
    -------
    str
        full path to installed data file

    Examples
    --------
        ecal_veto.bdt_file = makeBDTPath( 'gabrielle' )
    """

    full_path = '@CMAKE_INSTALL_PREFIX@/data/Ecal/' + bdt_name + '.onnx'
    if not os.path.isfile( full_path ) :
        print('ERROR: ONNX model file \'%s\' does not exist.' % ( full_path ))
        sys.exit(1)

    return full_path

def makeRoCPath( roc_name ) :
    """Get the full path to the RoC csv file

    Exits entire python script if file does not exist.

    Parameters
    ----------
    roc_name : str
        Name of RoC file to make path for (no extension)

    Returns
    -------
    str
        full path to installed data file

    Examples
    --------
        ecal_veto.roc_file = makeRoCPath( 'RoC_v14_8gev' )
    """

    full_path = '@CMAKE_INSTALL_PREFIX@/data/Ecal/' + roc_name + '.csv'
    if not os.path.isfile( full_path ) :
        print('ERROR: RoC csv file \'%s\' does not exist.' % ( full_path ))
        sys.exit(1)

    return full_path
