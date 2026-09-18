"""One downward 4 GeV muon for LDMX geometry development.

Use COSMIC_HCAL_ONLY=1 for just the three segmented stations. Sensitive detectors are
disabled: the legacy HCal readout model does not describe this CAD layout.
The full CAD-registered detector has not passed global overlap clearance.
"""

import os
from pathlib import Path

from LDMX.Framework import ldmxcfg
from LDMX.SimCore import generators, simulator


here = Path(__file__).resolve().parent
geometry = here.parents[1] / "data/ldmx-esa-cosmic-cad-v1"
output = Path.cwd()
os.chdir(geometry)  # Source-checkout GDML references neighboring subsystem files.
p = ldmxcfg.Process("cosmic_cad")
p.max_events = 1
p.run = 91826
p.output_files = [str(output / "cosmic_cad.root")]
sim = simulator.Simulator(instance_name="cosmic_cad")
sim.detector = str(
    geometry
    / (
        "hcal_only.gdml"
        if os.environ.get("COSMIC_HCAL_ONLY") == "1"
        else "detector.gdml"
    )
)
sim.scoring_planes = ""
sim.validate_detector = os.environ.get("COSMIC_CHECK_OVERLAPS") == "1"
sim.description = "CAD-registered cosmic stand geometry development; one selected muon"
# HcalSD also needs HcalGeometry for bar-local response coordinates, even with
# packed copy numbers. Do not silently attach a mismatched legacy provider.
sim.sensitive_detectors = []

gun = generators.Gun(
    instance_name="cosmic_probe",
    particle="mu-",
    energy=4.0,
    position=[5027.519993117254, -156.150230102, 4000.0],
    direction=[0.0, 0.0, -1.0],
)
gun.beam_spot_smear = [0.0, 0.0, 0.0]
sim.generators = [gun]
p.sequence = [sim]
