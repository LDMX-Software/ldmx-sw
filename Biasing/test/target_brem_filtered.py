from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("target_brem_filtered")
import LDMX.Ecal.ecal_geometry
import LDMX.Hcal.hcal_geometry
from LDMX.Biasing import filters
from LDMX.SimCore import generators, simulator


detector = "ldmx-det-v15-8gev"
generator = generators.single_8gev_e_upstream_tagger()

sim = simulator.Simulator(instance_name="target_brem")
sim.set_detector(detector, include_scoring_planes_minimal=True)
sim.description = "Target brem with angular and separation cuts"
sim.generators = [generator]

# Only keep events with a hard brem in the target,
# constrained by theta and delta-R cuts.
sim.actions.extend(
    [
        filters.TaggerVetoFilter(threshold=0.95 * 8000.0),
        filters.TargetBremFilter(
            recoil_max_p_threshold = 1500.0,
            brem_min_energy_threshold = 1000.0,
            brem_theta_min = 0.1,
            brem_theta_max = 999.,
            dral_min = 0.0,
            dral_max = 5.0,
        ),
    ]
)

p.sequence = [sim]
#p.logger.custom("biasing::TargetBremFilter", level = -1)
p.max_events = 100
p.max_tries_per_event = 1000000
p.output_files = ["target_brem_filtered.root"]
