

from ._differ import Differ
from ._plotter import plotter
import logging

log = logging.getLogger('photonuclear')

@plotter
def pndqm(d: Differ, out_dir=None):
    pn_vertex_volume_labels= ['Did not happen', 'Else', 'W Cooling', 'C Cooling', 'PCB',
       'CarbonBasePlate', 'Absorber', 'Sensor', 'Glue', 'Motherboard', '', '', '',]

    d.plot1d("PN/PN_event_type",  "Event category (200 MeV cut)",
             out_dir=out_dir,
             density=True
             )
    d.plot1d("PN/PN_event_type_500mev",  "Event category (500 MeV cut)",
             out_dir=out_dir,
             density=True
             )
    d.plot1d("PN/PN_event_type_2000mev",  "Event category (2000 MeV cut)",
             out_dir=out_dir,
             density=True)
    d.plot1d("PN/PN_event_type_compact", "", out_dir=out_dir)
    d.plot1d("PN/PN_event_type_compact_500mev", "", out_dir=out_dir)
    d.plot1d("PN/PN_event_type_compact_2000mev", "", out_dir=out_dir)
    d.plot1d("PN/PN_1n_event_type", "", out_dir=out_dir) 
    d.plot1d("PN/PN_pn_interaction_material", "", out_dir=out_dir)
    d.plot1d("PN/PN_pn_vertex_volume", "", out_dir=out_dir)
    d.plot1d("PN/PN_pn_particle_mult", "Photo-nuclear Multiplicity", out_dir=out_dir)
    d.plot1d("PN/PN_pn_neutron_mult", "Photo-nuclear Neutron Multiplicity", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_energy", "γ Energy [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_total_ke"  , "Total Kinetic Energy of Photo-Nuclear Products [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_total_neutron_ke"  , "Total Kinetic Energy of Photo-Nuclear Neutrons  [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1n_neutron_energy", "Neutron Energy [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1n_energy_diff", "E(γ_{PN}) - E(n) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1n_energy_frac", "E(n)/E(γ_{PN}) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_2n_n2_energy", "Energy of second hardest neutron [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_2n_energy_frac", "E(n)/E(γ_{PN}) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_2n_energy_other", "E_{other} [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy", "Charged Kaon Energy [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy_diff", "E(γ_{PN}) - E(K±) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1kp_energy_frac", "E(K±)/E(γ_{PN}) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy", "K0 Energy [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy_diff", "E(γ_{PN}) - E(K0) [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_1k0_energy_frac", "E(K0)/E(γ_{PN}) [MeV]", out_dir=out_dir)

    d.plot1d("PN/PN_recoil_vertex_x",   "Recoil e^{-} Vertex - x [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_recoil_vertex_y",   "Recoil e^{-} Vertex - y [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_recoil_vertex_z",   "Recoil e^{-} Vertex - z [mm]", out_dir=out_dir)

    d.plot1d("PN/PN_pn_gamma_int_x",    "γ Interaction Vertex X [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_int_y",    "γ Interaction Vertex Y [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_int_z",    "γ Interaction Vertex Z [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_z", "γ Vertex [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_x", "γ Vertex [mm]", out_dir=out_dir)
    d.plot1d("PN/PN_pn_gamma_vertex_y", "γ Vertex [mm]", out_dir=out_dir)

    d.plot1d("PN/PN_hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_theta",    "#theta of Hardest Photo-nuclear Particle  [Degrees]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton  [Degrees]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron  [Degrees]" )
    d.plot1d("PN/PN_hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi [MeV]", out_dir=out_dir)
    d.plot1d("PN/PN_hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi  [Degrees]", out_dir=out_dir)
