"""Helpers for locating trigger-scintillator data files installed by the build."""


def ts_data_path(name):
    """Absolute path to a file installed under data/TrigScint by setup_data.

    The install prefix is substituted by setup_python when the package is
    installed (the same mechanism LDMX.Tracking.rawdecoder.daq_map_path uses),
    so this resolves to the installed copy regardless of the working directory.
    """
    return "@CMAKE_INSTALL_PREFIX@/data/TrigScint/" + name


def ts_daq_map_path(name="ts_daqmap_esa25_slice_test.json"):
    """Absolute path to a TS DAQ-map JSON installed under data/TrigScint."""
    return ts_data_path(name)


def ts_calib_file_path(pad, period="esa25"):
    """Absolute path to the per-pad rechit calibration file (`ch gain ped` rows).

    `period` selects the data subdirectory: "esa25" holds the ESA25 slice-test
    calibrations, "esa26" the ESA26 cosmic-run ones (see
    TrigScint/data/esa26/README.md). They are vendored under TrigScint/data so
    they resolve inside the denv container (which mounts only the workspace).
    """
    return ts_data_path(f"{period}/calibration_pad{pad}.txt")
