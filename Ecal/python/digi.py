"""Package to configure the ECal digitization pipeline

All classes are derived versions of LDMX.Framework.ldmxcfg.Producer
with helpful member functions.

Two module-wide parameters are defined.

Attributes
----------
n_kelectrons_per_mip : float
    Number of thousand e-h pairs created for each MIP in 0.5mm thick Si
mip_si_energy : float
    Energy [MeV] of a single MIP on average in 0.5mm thick Si
"""

from LDMX.Framework import Processor, field, processor
from LDMX.Tools.hgcroc_emulator import HgcrocEmulator


n_kelectrons_per_mip = 37.0
"""thousand e-h pairs created per MIP

derived from 0.5mm thick Si
"""

charge_per_mip = n_kelectrons_per_mip * 0.1602
"""in fC, convering number of electrons to charge"""

mip_si_energy = 0.130
"""in MeV, corresponds to ~3.5 eV per e-h pair

derived from 0.5mm thick Si
"""

ecal_pulse_rate_up_slope = -0.345
ecal_pulse_time_up_slope = 70.6547
ecal_pulse_rate_dn_slope = 0.140068
ecal_pulse_time_dn_slope = 87.7649
ecal_pulse_time_peak = 77.732


def ecal_hgcroc_emulator():
    """Get an HGCROC emulator configured for the ECal specifically

    This sets the pulse shape parameters to the ones from a fit
    to a test readout of an ECal module and then thresholds to the
    default construction using 37k electrons as the number of
    electrons per MIP.
    """

    return HgcrocEmulator(
        rate_up_slope=ecal_pulse_rate_up_slope,
        time_up_slope=ecal_pulse_time_up_slope,
        rate_dn_slope=ecal_pulse_rate_dn_slope,
        time_dn_slope=ecal_pulse_time_dn_slope,
        time_peak=ecal_pulse_time_peak,
        n_adcs=10,
        i_soi=2,
    )


def calculate_energy_to_voltage_conversion(si_thickness: float):
    """calculate the conversion from a simulated energy [MeV] to voltage [mV]

     energy [MeV] * (thousand electrons per MIP)
                  * (charge per thousand electrons fC)
                  * (avg pad capacitance pF = 1/20)
                  * ( 1 MIP / energy [MeV] )
                  = voltage [mV]

    this leads to ~ 470 mV/MeV or ~6.8 MeV maximum hit (if 320 fC is max ADC range)

    Parameters
    ----------
    si_thickness : float
        thickness of silicon sensitive layers in mm
    """
    return charge_per_mip / 20.0 / (mip_si_energy * si_thickness / 0.5)


ecal_avg_gain: float = 0.3125 / 20.0


@processor("ecal::EcalDigiProducer", "Ecal")
class EcalDigiProducer(Processor):
    """Configuration for EcalDigiProducer

    Attributes
    ----------
    hgcroc : HgcrocEmulator
        Configuration for the chip emulator
    mev : float
        Conversion between energy [MeV] and voltage [mV]
        Depends on silicon thickness in simulation relative to the 0.5mm
        thickness that the physical constants were derived from.
        Default uses a silicon thickness of 0.4mm
    avg_readout_threshold : float
        Average readout threshold for all channels [mV], for noise emulation
    avg_pedestal : float
        Average pedestal for all channels [mV], for noise emulation
    avg_noise_rms: float
        Average noise RMS for all channels [mV], for noise emulation in empty channels
        Default is too optimistic, but need to mimic old noise model
    zero_suppresion : bool
        Should we suppress pure noise "hits" below readout threshold?
    input_coll_name : str
        Name of simulated ecal hits to digitize
    input_pass_name : str
        Name of pass to digitize
    digi_coll_name : str
        Output name of digis put into event bus
    """

    hgcroc: HgcrocEmulator = field(default_factory=ecal_hgcroc_emulator)
    mev: float = calculate_energy_to_voltage_conversion(0.4)
    avg_readout_threshold: float = 53.0 * ecal_avg_gain
    avg_pedestal: float = 50 * ecal_avg_gain
    avg_noise_rms: float = 0.6 * ecal_avg_gain
    zero_suppression: bool = True
    input_coll_name: str = "EcalSimHits"
    input_pass_name: str = ""
    digi_coll_name: str = "EcalDigis"


latest_second_order_energy_correction = 8000.0 / 7332.8
latest_layer_weights = [
    1.743,
    3.401,
    5.610,
    6.715,
    7.820,
    10.030,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    11.135,
    15.555,
    18.869,
    18.869,
    18.869,
    18.869,
    18.869,
    18.869,
    18.869,
    9.478,
]


@processor("ecal::EcalRecProducer", "Ecal")
class EcalRecProducer(Processor):
    """Configuration for the EcalRecProducer

    The layer weights and second order energy correction
    change when the ECal geometry changes, so we have setup
    various options for the different possible ECal geometries
    and their associated layer weights.

    Attributes
    ----------
    mip_si_energy : float
        Copied from module-wide mip_si_energy [MeV]
    charge_per_mip float
        Copied from module-wide charge_per_mip [fC]
    clock_cycle : float
        Time for one DAQ clock cycle to pass [ns]
    digi_coll_name : str
        Name of digi collection
    digi_pass_name : str
        Name of digi pass
    sim_hit_coll_name : str
        Name of sim collection to check for pure noise hit ID
    sim_hit_pass_name : str
        Name of sim pass
    rec_hit_coll_name : str
        Name of output collection
    second_order_energy_correction : float
        Correction to weighted energy, default to v15 geometry
    layer_weights : list of floats
        Weighting factors depending on layer index, default to v15 geometry
    """

    layer_weights: list[float] = latest_layer_weights
    second_order_energy_correction: float = latest_second_order_energy_correction
    mip_si_energy: float = mip_si_energy
    charge_per_mip: float = charge_per_mip
    clock_cycle: float = 25.0
    digi_coll_name: str = "EcalDigis"
    digi_pass_name: str = ""
    sim_hit_coll_name: str = "EcalSimHits"
    sim_hit_pass_name: str = ""
    rec_hit_coll_name: str = "EcalRecHits"

    @staticmethod
    def v2(**kwargs):
        """These layerWeights and energy correction were calculated at least
        before v3 geometry.

        The second order energy correction is determined by comparing the mean of
        1M single 4GeV electron events with 4GeV.
        """
        return EcalRecProducer(
            second_order_energy_correction=0.948,
            layer_weights=[
                1.641,
                3.526,
                5.184,
                6.841,
                8.222,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                8.775,
                12.642,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                16.51,
                8.45,
            ],
            **kwargs,
        )

    @staticmethod
    def v9(**kwargs):
        """These layerWeights and energy correction were calculated for the
        v9 geometry.

        The second order energy correction is determined by comparing the mean of
        1M single 4GeV electron events with 4GeV.
        """
        return EcalRecProducer(
            second_order_energy_correction=4000.0 / 4012.0,
            layer_weights=[
                1.019,
                1.707,
                3.381,
                5.022,
                6.679,
                8.060,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                8.613,
                12.480,
                16.347,
                16.347,
                16.347,
                16.347,
                16.347,
                16.347,
                16.347,
                16.347,
                16.347,
                8.334,
            ],
            **kwargs,
        )

    @staticmethod
    def v12(**kwargs):
        """These layerWeights and energy correction were calculated for the v12
        geometry.

        The second order energy correction is determined by comparing the mean of
        1M single 4GeV electron events with 4GeV.
        """
        return EcalRecProducer(
            second_order_energy_correction=4000.0 / 4007.0,
            layer_weights=[
                1.675,
                2.724,
                4.398,
                6.039,
                7.696,
                9.077,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                9.630,
                13.497,
                17.364,
                17.364,
                17.364,
                17.364,
                17.364,
                17.364,
                17.364,
                17.364,
                17.364,
                8.990,
            ],
            **kwargs,
        )

    @staticmethod
    def v14(**kwargs):
        """Generated for the v14 geometry

        The secondOrderEnergyCorrection is deteremined by generating 1M single 4GeV
        or 8GeV electron events shot directly into the front of the ECal from
        immediately upstream. The mean of the resulting total recon energy is found
        by fitting a two-sided normal distribution (one mean, a low and high deviation)
        to the histogram.
        """

        return EcalRecProducer(
            # second_order_energy_correction = 4000. / 3940.5,
            second_order_energy_correction=8000.0 / 7998.3,
            # these layer weights were the 'dE' column of the table output
            # by Detectors/util/ecal_layer_stack.py
            # See https://github.com/LDMX-Software/ldmx-sw/issues/1725
            # TLDR: these are wrong but only off by an absolute value of ~0.2,
            # future detector versions use the newer script with fixed material
            # properties
            layer_weights=[
                2.329,
                4.339,
                6.495,
                7.490,
                8.595,
                10.253,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                10.915,
                14.783,
                18.539,
                18.539,
                18.539,
                18.539,
                18.539,
                18.539,
                18.539,
                18.539,
                18.539,
                9.938,
            ],
            **kwargs,
        )

    @staticmethod
    def v15(**kwargs):
        """Generated for the v15 geometry

        The secondOrderEnergyCorrection is deteremined by generating 1M single
        4GeV or 8GeV electron events shot directly into the front of the ECal from
        immediately upstream. The mean of the resulting total recon energy is found
        by fitting a two-sided normal distribution (one mean, a low and high deviation)
        to the histogram.
        """
        return EcalRecProducer(
            second_order_energy_correction=latest_second_order_energy_correction,
            # these layer weights were the 'dE' column of the table output by
            # Detectors/util/ecal_layer_stack.py
            layer_weights=latest_layer_weights,
            **kwargs,
        )

    @staticmethod
    def v16(**kwargs):
        """Generated for the v16 geometry (Ecal is same as v15)

        The secondOrderEnergyCorrection is deteremined by generating 1M single
        4GeV or 8GeV electron events shot directly into the front of the ECal from
        immediately upstream. The mean of the resulting total recon energy is found
        by fitting a two-sided normal distribution (one mean, a low and high deviation)
        to the histogram.
        """
        return EcalRecProducer(
            second_order_energy_correction=latest_second_order_energy_correction,
            # these layer weights were the 'dE' column of the table output by
            # Detectors/util/ecal_layer_stack.py
            layer_weights=latest_layer_weights,
            **kwargs,
        )

    @staticmethod
    def reduced_v2(**kwargs):
        """Generated for the reduced v2 geometry

        TODO: The secondOrderEnergyCorrection for this geometry has yet to be
        calculated, so unity is being used as a placeholder.
        """

        return EcalRecProducer(
            second_order_energy_correction=1.0,
            layer_weights=[2.312, 5.417, 9.837, 11.910, 11.910, 11.910],
            **kwargs,
        )
