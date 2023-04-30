

from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('photonuclear')

@plotter(hist=True, event=False)
def pndqm(d: Differ, out_dir=None):
    event_type_labels = ['', 'Nothing hard', 'n', 'nn', '≥ 3n', 'π', 'ππ',
               'π₀', 'πA', 'π2A', 'ππA', 'π₀A',
               'π₀2A', 'π₀πA', 'p', 'pp', 'pn', 'K_L⁰X', 'KX',
                         'K_S⁰X', 'exotics', 'multi-body', '', '', '']

    compact_event_type_labels = ['', 'n', 'K^{±}X', 'K⁰', 'nn', 'soft', 'other', '','']
    neutron_event_type_labels = ['', '', 'nn', 'pn', 'π^+n', 'π⁰n', '', '']

    d.plot1d("PN/PN_event_type", "",
             tick_labels=event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_event_type_500mev", "",
             tick_labels=event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_event_type_2000mev", "",
             tick_labels=event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_event_type_compact", "",
             tick_labels=compact_event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_event_type_compact_500mev", "",
             tick_labels=compact_event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_event_type_compact_2000mev", "",
             tick_labels=compact_event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_1n_event_type", "",
             tick_labels=neutron_event_type_labels,
             out_dir=out_dir)
    d.plot1d("PN/PN_pn_particle_mult", "Photo-nuclear Multiplicity", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_energy", "γ Energy (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1n_neutron_energy", "Neutron Energy (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1n_energy_diff", "E(γ_{PN}) - E(n) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1n_energy_frac", "E(n)/E(γ_{PN}) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_2n_n2_energy", "Energy of second hardest neutron (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_2n_energy_frac", "E(n)/E(γ_{PN}) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_2n_energy_other", "E_{other} (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy", "Charged Kaon Energy (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy_diff", "E(γ_{PN}) - E(K±) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy_frac", "E(K±)/E(γ_{PN}) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy", "K0 Energy (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy_diff", "E(γ_{PN}) - E(K0) (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy_frac", "E(K0)/E(γ_{PN}) (MeV)", out_dir=out_dir)

    d.plot1d("PN/PN_recoil_vertex_x",   "Recoil e^{-} Vertex - x (mm)", out_dir=out_dir)
    d.plot1d("PN/PN_recoil_vertex_y",   "Recoil e^{-} Vertex - y (mm)", out_dir=out_dir)
    d.plot1d("PN/PN_recoil_vertex_z",   "Recoil e^{-} Vertex - z (mm)", out_dir=out_dir)

    d.plot1d("PN/PN_pn_gamma_int_z",    "γ Interaction Vertex (mm)", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_z", "γ Vertex (mm)", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_x", "γ Vertex (mm)", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_y", "γ Vertex (mm)", out_dir=out_dir)

    d.plot1d("PN/PN_hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_theta",    "#theta of Hardest Photo-nuclear Particle (Degrees)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton (Degrees)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron (Degrees)" )
    d.plot1d("PN/PN_hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi (MeV)", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi (Degrees)", out_dir=out_dir)
