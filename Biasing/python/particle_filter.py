""" Example configurations for filtering out PN events by the properties of 
    of the reaction products.
"""

from LDMX.SimCore import simcfg

class PhotoNuclearProductsFilter(simcfg.UserAction) :
    """ Configuration for keeping events with specific products of PN interactions

    Parameters
    ----------
    name : str
        Name for this filter

    Attributes
    ----------
    pdg_ids : list of ints
        List of PDG product IDs to look for in PN products
    """

    def __init__(self,name) :
        super().__init__(name,'biasing::PhotoNuclearProductsFilter')

        from LDMX.Biasing import include
        include.library()

        self.pdg_ids = [ ]

    def kaon() :
        """ Configuration for filtering photo-nuclear events whose products don't contain a kaon.
    
    
        Returns
        -------
        Instance of configured PhotoNuclearProductsFilter
        """
    
        particle_filter = PhotoNuclearProductsFilter("kaon_filter")
        particle_filter.pdg_ids = [
                130, # K_L^0
                310, # K_S^0
                311, # K^0
                321  # K^+
        ]
        return particle_filter

class PhotoNuclearTopologyFilter(simcfg.UserAction):
    """Configuration for keeping events with a PN reaction producing a
    particular event topology.

    Parameters
    ----------
    name : str
        Name for this filter
    filter_class:
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
    def __init__(self, name, filter_class):
        super().__init__(name, filter_class)
        from LDMX.Biasing import include
        include.library()
        self.count_light_ions = True


    def SingleNeutronFilter():
        """Configuration for keeping photonuclear events where a single neutron
        carries most of the kinetic energy from the interaction


        Returns
        -------
        Instance of configured PhotoNuclearTopologyFilter

        """
        filter = PhotoNuclearTopologyFilter(name='SingleNeutron',
                                            filter_class='biasing::SingleNeutronFilter' )
        filter.hard_particle_threshold = 200.
        return filter

    def NothingHardFilter():
        """Configuration for keeping photonuclear events no particles received
        significant kinetic energy.

        Returns
        -------
        Instance of configured PhotoNuclearTopologyFilter

        """
        filter = PhotoNuclearTopologyFilter(name='NothingHard',
                                            filter_class='biasing::NothingHardFilter')
        filter.hard_particle_threshold = 200.
        return filter



    

