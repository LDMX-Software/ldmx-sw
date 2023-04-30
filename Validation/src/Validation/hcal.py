
from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('hcal')

@plotter(hist=True, event=False)
def dqm(d: Differ, out_dir=None):
    sections = ['back', 'top', 'bottom', 'left', 'right']
    histogram_name_format = 'hcal_dqm_{0}/hcal_dqm_{0}_{1}'
    histograms = [
        ('pe', 'HCal PE ({})'),
        ('hit_time', 'HCal hit time ({}) [ns]'),
        ('sim_hit_time', 'HCal hit time ({}) [ns]'),
        ("layer", "Layer number ({})"),
        ("sim_layer", "Layer number ({})"),
        ("energy", "Total reconstructed energy in the HCal ({})"),
        ("sim_energy", "Total reconstructed energy in the HCal ({})"),
        ("sim_energy_per_bar", "Total reconstructed energy in the HCal ({})"),
        ("sim_total_energy", "Total reconstructed energy in the HCal ({})"),
        ("total_pe", "Total photoelectrons in the HCal ({})"),
        ('max_pe', "Maximum photoelectrons in the HCal ({})"),
        ("hit_multiplicity", "HCal hit multiplicity ({})"),
        ("sim_hit_multiplicity", "HCal hit multiplicity ({})"),
        ("sim_num_bars_hit", "HCal hit multiplicity ({})"),
        ("vetoable_hit_multiplicity", "Multiplicity of vetoable hits ({})"),
        ('max_pe_time', "Max PE hit time ({}) [ns]",),
        ('along_x', 'x'),
        ('along_y', 'y'),
        ('along_z', 'z'),
        ('sim_along_x', 'x'),
        ('sim_along_y', 'y'),
        ('sim_along_z', 'z'),
    ]

    for section in sections:
        for histogram, title in histograms:
            name = histogram_name_format.format(section, histogram)
            title = title.format(section.capitalize())
            log.info(f"Making the {name} histogram...")
            d.plot1d(name, title, out_dir=out_dir)
        d.plot1d(f'HcalInefficiencyAnalyzer/HcalInefficiencyAnalyzer_inefficiency_{section}',
                 'Inefficiency',
                 out_dir=out_dir)



