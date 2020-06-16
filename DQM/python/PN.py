"""Configured PhotoNuclearDQM python object

Contains an instance of PhotoNuclearDQM that
has already been configured.

Builds the necessary histograms as well.

Examples
--------
    from LDMX.DQM.PN import photo_nuclear
"""


from LDMX.Framework import ldmxcfg

photo_nuclear = ldmxcfg.Analyzer("Photo-nuclear DQM", "ldmx::PhotoNuclearDQM")

photo_nuclear.build1DHistogram("event_type"         , "", 24, -1, 23)
photo_nuclear.build1DHistogram("event_type_500mev"  , "", 24, -1, 23)
photo_nuclear.build1DHistogram("event_type_2000mev" , "", 24, -1, 23)
photo_nuclear.build1DHistogram("event_type_compact"         , "", 8, -1, 7)
photo_nuclear.build1DHistogram("event_type_compact_500mev"  , "", 8, -1, 7)
photo_nuclear.build1DHistogram("event_type_compact_2000mev" , "", 8, -1, 7)
photo_nuclear.build1DHistogram("1n_event_type"      , "", 7,  -1, 6)
photo_nuclear.build1DHistogram("pn_particle_mult"   , "Photo-nuclear Multiplicity", 100, 0, 200)
photo_nuclear.build1DHistogram("pn_gamma_energy"    , "#gamma Energy (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1n_neutron_energy"  , "Neutron Energy (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1n_energy_diff"     , "E(#gamma_{PN}) - E(n) (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1n_energy_frac"     , "E(n)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
photo_nuclear.build1DHistogram("2n_n2_energy"       , "Energy of second hardest neutron (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("2n_energy_frac"     , "E(n)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
photo_nuclear.build1DHistogram("2n_energy_other"    , "E_{other} (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1kp_energy"         , "Charged Kaon Energy (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1kp_energy_diff"    , "E(#gamma_{PN}) - E(K#pm) (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1kp_energy_frac"    , "E(K#pm)/E(#gamma_{PN}) (MeV)", 500, 0, 1)
photo_nuclear.build1DHistogram("1k0_energy"         , "K0 Energy (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1k0_energy_diff"    , "E(#gamma_{PN}) - E(K0) (MeV)", 500, 0, 5000)
photo_nuclear.build1DHistogram("1k0_energy_frac"    , "E(K0)/E(#gamma_{PN}) (MeV)", 500, 0, 1)

photo_nuclear.build1DHistogram("recoil_vertex_x",   "Recoil e^{-} Vertex - x (mm)", 40, -40, 40)
photo_nuclear.build1DHistogram("recoil_vertex_y",   "Recoil e^{-} Vertex - y (mm)", 80, -80, 80)
photo_nuclear.build1DHistogram("recoil_vertex_z",   "Recoil e^{-} Vertex - z (mm)", 20, -750, -650)

photo_nuclear.build1DHistogram("pn_gamma_int_z",    "#gamma Interaction Vertex (mm)", 50, 200, 400)
photo_nuclear.build1DHistogram("pn_gamma_vertex_z", "#gamma Vertex (mm)", 1000, -5,  5)
photo_nuclear.build1DHistogram("pn_gamma_vertex_x", "#gamma Vertex (mm)", 80,   -40, 40)
photo_nuclear.build1DHistogram("pn_gamma_vertex_y", "#gamma Vertex (mm)", 160,  -80, 80)

photo_nuclear.build1DHistogram("hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle (MeV)", 400, 0, 4000)
photo_nuclear.build1DHistogram("hardest_theta",    "#theta of Hardest Photo-nuclear Particle (Degrees)", 360, 0, 180)
photo_nuclear.build1DHistogram("hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton (MeV)", 400, 0, 4000)
photo_nuclear.build1DHistogram("hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton (Degrees)", 360, 0, 180)
photo_nuclear.build1DHistogram("hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron (MeV)", 400, 0, 4000)
photo_nuclear.build1DHistogram("hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron (Degrees)", 360, 0, 180)
photo_nuclear.build1DHistogram("hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi (MeV)", 400, 0, 4000)
photo_nuclear.build1DHistogram("hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi (Degrees)", 360, 0, 180)

photo_nuclear.build2DHistogram("h_ke_h_theta", 
                    "Kinetic Energy Hardest Photo-nuclear Particle (MeV)",
                    400, 0, 4000, 
                    "#theta of Hardest Photo-nuclear Particle (Degrees)",
                    360, 0, 180)

photo_nuclear.build2DHistogram("1n_ke:2nd_h_ke", 
                    "Kinetic Energy of Leading Neutron (MeV)",
                    400, 0, 4000, 
                    "Kinetic Energy of 2nd Hardest Particle",
                    400, 0, 4000)

photo_nuclear.build2DHistogram("1kp_ke:2nd_h_ke", 
                    "Kinetic Energy of Leading Charged Kaon (MeV)",
                    400, 0, 4000, 
                    "Kinetic Energy of 2nd Hardest Particle",
                    400, 0, 4000)

photo_nuclear.build2DHistogram("1k0_ke:2nd_h_ke", 
                    "Kinetic Energy of Leading K0 (MeV)",
                    400, 0, 4000, 
                    "Kinetic Energy of 2nd Hardest Particle",
                    400, 0, 4000)

photo_nuclear.build2DHistogram("recoil_vertex_x:recoil_vertex_y", 
                   "Recoil electron vertex x (mm)", 
                   160, -40, 40, 
                   "Recoil electron vertex y (mm)", 
                   320, -80, 80)
