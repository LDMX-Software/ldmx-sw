from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")

p.max_tries_per_event = 10000

from LDMX.Biasing import ecal
from LDMX.SimCore import generators as gen
from LDMX.SimCore import photonuclear_models as pn
from LDMX.SimCore.user_actions import PhotonuclearTracker


det = "ldmx-det-v15-8gev"

my_sim = ecal.photo_nuclear(det, gen.single_8gev_e_upstream_tagger())
my_sim.description = "ECal PN with Bertini Cascade History Recording"

# Use BertiniWithHistoryModel to produce PhotonuclearCascadeHistories
my_sim.photonuclear_model = pn.BertiniWithHistoryModel()

# Enable detailed photonuclear interaction tracking
my_sim.actions.append(PhotonuclearTracker())

p.sequence = [my_sim]

##################################################################
# Below should be the same for all sim scenarios

import os


p.max_events = int(os.environ["LDMX_NUM_EVENTS"])
p.run = int(os.environ["LDMX_RUN_NUMBER"])

p.histogram_file = "hist.root"
p.output_files = ["events.root"]

# Load the ECAL modules
import LDMX.Ecal.ecal_geometry
import LDMX.Ecal.ecal_hardcoded_conditions

# Load the HCAL modules
import LDMX.Hcal.hcal_geometry
import LDMX.Hcal.hcal_hardcoded_conditions

# Load the DQM modules
from LDMX.DQM import dqm


p.logger.term_level = int(os.environ["LDMX_LOG_LEVEL"])
# Example to show trace level logging for ecal veto (only)
# p.logger.custom(ecal_veto, level = -1)

p.sequence.extend(
    [
        dqm.PhotoNuclearDQM(),
        dqm.CascadeHistoryDQM(),
    ]
)
