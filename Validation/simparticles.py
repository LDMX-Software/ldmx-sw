from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('8GeV')

@plotter
def sample_validation(d: Differ, out_dir=None):

    pdgid_labels = ['', 'e+', 'e-', 'μ+', 'μ-', 'γ', 'p', 'n', 'π+', 'π-', 'π0', 'K+', 'K-', 'K-L', 'K-S', 'light nucleus', 'heavy nucleus', 'strange baryon', "A\'", 'something else'] # finish later 

    d.plot1d("SampleValidation/SampleValidation_primaries_pdgid", "PDG ID, primaries",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_primaries_energy", "Energy of primaries [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_primarydaughters_pdgid", "PDG ID, primary daughters",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)
    d.plot1d("SampleValidation/SampleValidation_daughterphoton_energy", "Energy spectrum of all photons from primary [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_harddaughters_pdgid", "PDG ID of hard primary daughter",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_harddaughters_startZ", "Start z position of hard primary daughter [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_harddaughters_endZ", "End z position of hard primary daughter [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_harddaughters_energy", "Energy spectrum of hard primary daughter [MeV]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_hardbremdaughters_pdgid", "PDG ID, hard brem daughters",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_hardbremdaughters_startZ", "Start z position of hard brem daughters [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_hardbremdaughters_endZ", "End z position of hard brem daughters [mm]",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_hardbremdaughters_energy", "Energy of hard brem daughters [MeV]",
             out_dir=out_dir,
             density=True)
