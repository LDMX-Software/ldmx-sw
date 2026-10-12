"""Configuration for TrigScintGeometry

Parameters mirror the trigger-scintillator GDML constants and are served to reco
as the `trig_scint_geometry` conditions object (see TrigScintGeometry.cxx).
"""

from LDMX.Framework import field, parameter_set


@parameter_set
class TrigScintReadoutGeometry:
    """Bar geometry for one TS geometry version (all lengths in mm).

    Attributes
    ----------
    detectors_valid : list[str]
        Regexes selecting which detectors use this geometry.
    module_z : list[float]
        Per-module layer-pair center z (target-relative frame). Index = module.
    y0 : float
        Global y of bar 0 (bar 0 is most +y).
    layer_pitch : float
        In-layer y pitch (lp_bar_dy + lp_bar_y_gap in the gdml).
    layer_y_shift : float
        y stagger of layer 1 vs layer 0 (lp_layer_y_shift).
    layer_z_sep : float
        z separation of the two layers within a module (bar thickness + gap).
    n_bars : int
        Bars (channels) per module.
    """

    detectors_valid: list[str]
    module_z: list[float]
    y0: float
    layer_pitch: float
    layer_y_shift: float
    layer_z_sep: float
    n_bars: int

    def make_esa_v1():
        """esa25-v1 / ESA stand: pad1=-381, pad2=-305.8, pad3=-76.2 mm.

        NOTE: module_z ordering assumes decode Pad1/Pad2/Pad3 == geometry
        pad1/pad2/pad3; reorder if the DAQ cabling differs. LYSO (Pad4) not
        included yet.
        """
        return TrigScintReadoutGeometry(
            detectors_valid=[
                "ldmx-esa25-v1",
                "ldmx-esa.*",
            ],
            module_z=[-381.0, -305.8, -76.2],
            y0=18.875,
            layer_pitch=3.15,
            layer_y_shift=1.55,
            layer_z_sep=4.0,
            n_bars=24,
        )


@parameter_set
class TrigScintGeometry:
    """Container for the various TS geometries."""

    esa_v1: TrigScintReadoutGeometry = field(
        default_factory=TrigScintReadoutGeometry.make_esa_v1
    )
