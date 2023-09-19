#
from LDMX.Trigger import trigger_energy_sums

trigger_seq = [
    trigger_energy_sums.EcalTPSelector(),
    trigger_energy_sums.TrigEcalEnergySum(),
    trigger_energy_sums.TrigHcalEnergySum(),
    trigger_energy_sums.TrigEcalClusterProducer(),
]
