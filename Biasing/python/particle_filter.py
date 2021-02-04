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
