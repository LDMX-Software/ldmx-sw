
from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('hcal')

@plotter(hist=True, event=False)
def dqm(d: Differ, out_dir=None):
    sections = ['back', 'top', 'bottom', 'left', 'right']
    histogram_name_format = 'hcal_dqm_{0}/hcal_dqm_{0}_{1}'
    histograms = [
        ('pe', 'Reconstructed PE per hit ({})'),
        ('hit_time', 'HCal hit time ({}) [ns]'),
        ('sim_hit_time', 'HCal hit time ({}) [ns]'),
        ("layer", "Layer number ({})"),
        ("sim_layer", "Layer number ({})"),
        ("energy", "Reconstructed hit energy in the HCal ({})"),
        ("total_energy", "Total reconstructed energy in the HCal ({}) [MeV]"),
        ("sim_energy", "Total energy deposited in the HCal ({}) [MeV]"),
        ("sim_energy_per_bar", "Total energy deposited per bar in the HCal ({}) [MeV]"),
        ("sim_total_energy", "Total energy deposited in the HCal ({}) [MeV]"),
        ("total_pe", "Total photoelectrons in the HCal ({})"),
        ('max_pe', "Maximum photoelectrons in any HCal bar ({})"),
        ("hit_multiplicity", "HCal hit multiplicity ({})"),
        ("sim_hit_multiplicity", "HCal hit multiplicity ({})"),
        ("sim_num_bars_hit", "HCal hit multiplicity ({})"),
        ("vetoable_hit_multiplicity", "Multiplicity of vetoable hits (> 8PE) ({})"),
        ('max_pe_time', "Max PE hit time ({}) [ns]",),
        ('along_x', 'Reconstructed hit position along horizontal bars [mm]'),
        ('along_y', 'Reconstructed hit position along vertical bars [mm]'  ),
        ('along_z', 'Reconstructed hit position along z-oriented bars [mm]'),
        ('sim_along_x', 'Reconstructed hit position along horizontal bars [mm]'),
        ('sim_along_y', 'Reconstructed hit position along vertical bars [mm]'  ),
        ('sim_along_z', 'Reconstructed hit position along z-oriented bars [mm]'),
    ]

    log.info("Making the efficiency histogram...")
    d.plot1d('HcalInefficiencyAnalyzer/HcalInefficiencyAnalyzer_efficiency',
             'Hcal part involved in veto',
             'Efficiency',
             tick_labels=['', 'Back', 'Top', 'Bottom',
                          'Right', 'Left', 'Any',
                          'Both', 'Back only', 'Side only',
                          'Neither', ''],
             out_dir=out_dir,
             yscale='linear',
)

    for section in sections:
        for histogram, title in histograms:
            name = histogram_name_format.format(section, histogram)
            title = title.format(section.capitalize())
            log.info(f"Making the {name} histogram...")
            d.plot1d(name, title, out_dir=out_dir, density=True, ylabel='Event rate')
        log.info(f"Making the inefficiency figure for {section}")
        d.plot1d(f'HcalInefficiencyAnalyzer/HcalInefficiencyAnalyzer_inefficiency_{section}',
                 'Layer',
                 'Inefficiency (5PE)',
                 out_dir=out_dir,
                 density=True)



