"""Helper for locating the trigger-scintillator DAQ map JSON."""


def ts_daq_map_path(name="ts_daqmap_esa25_slice_test.json"):
    """Absolute path to a TS DAQ-map JSON installed under data/TrigScint.

    The install prefix is substituted by setup_python when the package is
    installed (the same mechanism LDMX.Tracking.rawdecoder.daq_map_path uses),
    so this resolves to the installed copy regardless of the working directory.
    """
    return "@CMAKE_INSTALL_PREFIX@/data/TrigScint/" + name
