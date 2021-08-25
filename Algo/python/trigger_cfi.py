#
from LDMX.Trigger import trigger_energy_sums

trigger_seq = [
    trigger_energy_sums.TriggerEcalEnergySum(),
    trigger_energy_sums.TriggerHcalEnergySum(),
    trigger_energy_sums.TrigEcalClusterProducer(),
]
