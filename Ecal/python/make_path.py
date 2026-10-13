"""Helpful python configuration functions for getting the path to installed data files.

This file was configured by cmake for the installation of ldmx-sw at
   @CMAKE_INSTALL_PREFIX@
"""

import os
import sys


def make_bdt_path(bdt_name):
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
        ecal_veto.bdt_file = make_bdt_path( 'gabrielle' )
    """

    full_path = "@CMAKE_INSTALL_PREFIX@/data/Ecal/" + bdt_name + ".onnx"
    if not os.path.isfile(full_path):
        print(f"ERROR: ONNX model file '{full_path}' does not exist.")
        sys.exit(1)

    return full_path


def make_roc_path(roc_name):
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
        ecal_veto.roc_file = make_roc_path( 'RoC_v14_8gev' )
    """

    full_path = "@CMAKE_INSTALL_PREFIX@/data/Ecal/" + roc_name + ".csv"
    if not os.path.isfile(full_path):
        print(f"ERROR: RoC csv file '{full_path}' does not exist.")
        sys.exit(1)

    return full_path


def make_ci_data_path(relative_path):
    """Get the full path to a file in the ci-data repository

    Exits entire python script if CI_DATA is not set or the file does not exist.

    Parameters
    ----------
    relative_path : str
        Path to the file relative to the top of the ci-data repository

    Returns
    -------
    str
        full path to the ci-data file

    Examples
    --------
        nn.model_path = make_ci_data_path( 'target_genie/ecal_nn_trigger.onnx' )
    """

    url = f"https://github.com/LDMX-Software/ci-data/raw/main/{relative_path}"
    ci_data = os.environ.get("CI_DATA")
    if ci_data is None:
        print(
            f"ERROR: CI_DATA is not set, cannot find '{relative_path}'. "
            f"Download it from {url} and pass its path explicitly."
        )
        sys.exit(1)

    full_path = os.path.join(ci_data, relative_path)
    if not os.path.isfile(full_path):
        print(
            f"ERROR: ci-data file '{full_path}' does not exist. Download it from {url}."
        )
        sys.exit(1)

    return full_path
