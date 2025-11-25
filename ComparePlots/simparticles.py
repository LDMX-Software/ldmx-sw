from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('8GeV')

@plotter
def sample_validation(d: Differ, out_dir=None):
    d.plot1d("SampleValidation/primaries_pdgid", "PDG ID, primaries",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/primaries_energy", "Energy of primaries [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/primarydaughters_pdgid", "PDG ID, primary daughters",
             out_dir=out_dir,
             density=True)
    d.plot1d("SampleValidation/daughterphoton_energy", "Energy spectrum of all photons from primary [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/harddaughters_pdgid", "PDG ID of hard primary daughter",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/harddaughters_startZ", "Start z position of hard primary daughter [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/harddaughters_endZ", "End z position of hard primary daughter [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/harddaughters_energy", "Energy spectrum of hard primary daughter [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/hardbremdaughters_pdgid", "PDG ID, hard brem daughters",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/hardbremdaughters_startZ", "Start z position of hard brem daughters [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/hardbremdaughters_endZ", "End z position of hard brem daughters [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/hardbremdaughters_energy", "Energy of hard brem daughters [MeV]",
             out_dir=out_dir,
             density=True)
