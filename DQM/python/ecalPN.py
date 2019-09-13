
from LDMX.Framework import ldmxcfg

ecalPN = ldmxcfg.Analyzer("ECalPN", "ldmx::EcalPN")
ecalPN.parameters["ecal_veto_collection"] = "EcalVeto"

titles = ['', '_track_veto', '_bdt', '_hcal', '_track_bdt', '_vetoes']
for t in titles: 
    ecalPN.build1DHistogram("event_type%s"         % t, "", 24, -1, 23)
    ecalPN.build1DHistogram("event_type_500mev%s"  % t, "", 24, -1, 23)
    ecalPN.build1DHistogram("event_type_2000mev%s" % t, "", 24, -1, 23)
    ecalPN.build1DHistogram("1n_event_type%s"      % t, "", 7,  -1, 6)
    ecalPN.build1DHistogram("pn_particle_mult%s"   % t, "Photo-nuclear Multiplicity", 100, 0, 200)
    ecalPN.build1DHistogram("pn_gamma_energy%s"    % t, "#gamma Energy (MeV)", 500, 0, 5000)
    ecalPN.build1DHistogram("1n_neutron_energy%s"  % t, "Neutron Energy (MeV)", 500, 0, 5000)
    ecalPN.build1DHistogram("1n_energy_diff%s"     % t, "E(#gamma_{PN}) - E(n) (MeV)", 500, 0, 5000)
    ecalPN.build1DHistogram("1n_energy_frac%s"     % t, "E(n)/E(#gamma_{PN}) (MeV)", 500, 0, 1)

ecalPN.build1DHistogram("pn_gamma_int_z",    "#gamma Interaction Vertex (mm)", 50, 200, 400)
ecalPN.build1DHistogram("pn_gamma_vertex_z", "#gamma Vertex (mm)", 100, -1, 1)

ecalPN.build1DHistogram("hardest_ke",       "Kinetic Energy Hardest Photo-nuclear Particle (MeV)", 400, 0, 4000)
ecalPN.build1DHistogram("hardest_theta",    "#theta of Hardest Photo-nuclear Particle (Degrees)", 360, 0, 180)
ecalPN.build1DHistogram("hardest_p_ke",     "Kinetic Energy Hardest Photo-nuclear Proton (MeV)", 400, 0, 4000)
ecalPN.build1DHistogram("hardest_p_theta",  "#theta of Hardest Photo-nuclear Proton (Degrees)", 360, 0, 180)
ecalPN.build1DHistogram("hardest_n_ke",     "Kinetic Energy Hardest Photo-nuclear Neutron (MeV)", 400, 0, 4000)
ecalPN.build1DHistogram("hardest_n_theta",  "#theta of Hardest Photo-nuclear Neutron (Degrees)", 360, 0, 180)
ecalPN.build1DHistogram("hardest_pi_ke",    "Kinetic Energy Hardest Photo-nuclear #pi (MeV)", 400, 0, 4000)
ecalPN.build1DHistogram("hardest_pi_theta", "#theta of Hardest Photo-nuclear #pi (Degrees)", 360, 0, 180)

