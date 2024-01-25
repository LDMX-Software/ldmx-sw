from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('8GeV')

@plotter(hist=True, event=False)
def beamenergy_comp(d: Differ, out_dir=None):

    pdgid_labels = ['', 'e+', 'e-', 'μ+', 'μ-', 'γ', 'p', 'n', 'π+', 'π-', 'π0', 'K+', 'K-', 'K-L', 'K-S', 'light nucleus', 'heavy nucleus', 'strange baryon', "A\'", 'something else'] # finish later 

    d.plot1d("SampleValidation/SampleValidation_pdgid_primaries", "PDG ID, primaries",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_energy_primaries", "Energy of primaries (MeV)",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_pdgid_primarydaughters", "PDG ID, primary daughters",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)
    d.plot1d("SampleValidation/SampleValidation_energy_daughterphoton", "Energy spectrum of all photons from primary",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_pdgid_harddaughters", "PDG ID of hard primary daughter",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_startZ_hardbrem", "Start z position of hard primary daughter",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_endZ_hardbrem", "End z position of hard primary daughter",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_energy_hardbrem", "Energy spectrum of hard primary daughter",
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_pdgid_hardbremdaughters", "PDG ID, hard brem daughters",
             tick_labels=pdgid_labels,
             out_dir=out_dir,
             density=True)

    d.plot1d("SampleValidation/SampleValidation_startZ_hardbremdaughters", "Start z position of hard brem daughters",
             out_dir=out_dir,
             density=True)
