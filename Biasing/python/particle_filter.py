"""PN topology filtering user actions"""

from LDMX.SimCore.user_actions import UserAction, user_action


@user_action("biasing::PhotoNuclearProductsFilter", "Biasing")
class PhotoNuclearProductsFilter(UserAction):
    """Configuration for keeping events with specific products of PN interactions

    Attributes
    ----------
    pdg_ids : list of ints
        List of PDG product IDs to look for in PN products
    min_e: float
        minimum energy
    """

    pdg_ids: list[int] = []
    min_e: float = 0.0

    def kaon():
        """Configuration for filtering photo-nuclear events whose products don't
        contain a kaon.


        Returns
        -------
        Instance of configured PhotoNuclearProductsFilter
        """

        return PhotoNuclearProductsFilter(
            instance_name="kaon_filter",
            pdg_ids=[
                130,  # K_L^0
                310,  # K_S^0
                311,  # K^0
                321,  # K^+
            ],
        )


@user_action("UNDEFINED", "Biasing")
class PhotoNuclearTopologyFilter(UserAction):
    """Configuration for keeping events with a PN reaction producing a
    particular event topology.

    Parameters
    ----------
    instance_name : str
        Name for this filter
    class_name:
        Name of the class that implements this filter. Should correspond to the
        invocation of DECLARE_ACTION in PhotoNuclearTopologyFilters.cxx

    Attributes
    ----------
    hard_particle_threshold: float
        The kinetic energy threshold required to count a particle as "hard"

    count_light_ions: bool
        Whether or not light ions (e.g. deutrons) should be considered when
        applying the filter. Setting this to False can produce misleading
        results such as classifying events with very high kinetic energy light
        ions as "Nothing Hard" but it is the current default choice in the
        PhotoNuclearDQM.

    """

    hard_particle_threshold: float
    count_light_ions: bool = True

    def single_neutron():
        """Configuration for keeping photonuclear events where a single neutron
        carries most of the kinetic energy from the interaction


        Returns
        -------
        Instance of configured PhotoNuclearTopologyFilter

        """
        return PhotoNuclearTopologyFilter(
            instance_name="SingleNeutron",
            class_name="biasing::SingleNeutronFilter",
            hard_particle_threshold=200.0,
        )

    def nothing_hard():
        """Configuration for keeping photonuclear events no particles received
        significant kinetic energy.

        Returns
        -------
        Instance of configured PhotoNuclearTopologyFilter

        """
        return PhotoNuclearTopologyFilter(
            instance_name="NothingHard",
            class_name="biasing::NothingHardFilter",
            hard_particle_threshold=200.0,
        )
