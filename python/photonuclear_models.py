"""Configuration classes for default photonuclear models"""

from LDMX.SimCore import simcfg


class BertiniModel(simcfg.PhotoNuclearModel):
    """The default model for photonuclear interactions.

    Keeps the default Bertini model from Geant4.
    """

    def __init__(self):
        super().__init__('BertiniModel',
                         'simcore::BertiniModel',
                         'SimCore_PhotoNuclearModels')

class BertiniNothingHardModel(simcfg.PhotoNuclearModel):
    """A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self):
        """
        Nothing hard events are unlikely to come from low A nuclei. This can
        be tested by instrumenting one of the models and checking the typical
        number of attempts for different nuclei.

        If we count light ions and nuclei as potential hard particles and
        ignore events producing heavy exotic particles, then here are some
        example results:

        Z74 -> ~5000 attempts

        While for lighter nuclei, perhaps unsurprsingly
        Z1  -> 1e5 attempts before giving up
        Z6  -> 1e5 attempts before giving up
        Z7  -> 1e5 attempts before giving up
        Z8  -> 1e5 attempts before giving up
        Z11 -> 1e5 attempts before giving up
        Z14 -> 1e5 attempts before giving up
        Z20 -> 1e5 attempts before giving up
        Z29 -> 30000 attempts

        In a similar manner, we can check how often a photonuclear interaction
        is with a particular nucleus in an ECal PN sample

        |Nucleus | Rate [%] |
        |  H     |     4.78 |
        |  C     |     9.73 |
        |  N     |     0.04 |
        |  O     |     7.01 |
        |  Ni    |     1.25 |
        |  Al    |     0.01 |
        |  Si    |     6.23 |
        |  Ca    |     1.36 |
        |  Mn    |     0.01 |
        |  Fe    |     0.67 |
        |  Cu    |    12.47 |
        |  W     |    56.44 |

        So for the purposes of running Nothing hard simulations, it is
        probably fine to have a high `zmin` value, either 29 (Cu) or 74 (W).
        Single hard particle events, however, can come from any kind of
        nucleus.
        """

        super().__init__('BertiniNothingHardModel',
                         'simcore::BertiniNothingHardModel',
                         'SimCore_PhotoNuclearModels')
        self.count_light_ions = True
        self.hard_particle_threshold = 200.
        self.zmin = 74
        self.emin = 2500.
class BertiniSingleNeutronModel(simcfg.PhotoNuclearModel):
    """A photonuclear model producing only topologies where only one neutron has
    kinetic energy above a particular threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self):
        super().__init__('BertiniSingleNeutronModel',
                         'simcore::BertiniSingleNeutronModel',
                         'SimCore_PhotoNuclearModels')
        self.hard_particle_threshold = 200.
        self.zmin = 0
        self.emin = 2500.
        self.count_light_ions = True



class BertiniAtLeastNProductsModel(simcfg.PhotoNuclearModel):
    """ A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    def __init__(self, name):
        super().__init__(name,
                         'simcore::BertiniAtLeastNProductsModel',
                         'SimCore_PhotoNuclearModels')
        self.hard_particle_threshold = 200.
        self.zmin = 0
        self.emin = 2500.
        self.min_products = 1
        self.pdg_ids = []

    def kaon(min_products = 1, hard_particle_threshold=200.):
        # Note: By default, this is requiring at least 1 kaon with at least 200
        # MeV. You may want a different energy threshold depending on your needs.
        model = BertiniAtLeastNProductsModel(f"{min_products}_kaon_model")
        model.hard_particle_threshold=hard_particle_threshold
        model.pdg_ids = [
                130,  # K_L^0
                310,  # K_S^0
                311,  # K^0
                321,  # K^+
                -321, # K^-
        ]
        model.min_products = min_products
        return model


class NoPhotoNuclearModel(simcfg.PhotoNuclearModel):
    """A PhotoNuclear model that disables the photonuclear process entirely.

    Make sure that no biasing operators for photonuclear reactions are enabled
    when using this model.

    """
    def __init__(self):
        super().__init__('NoPhotoNuclearModel',
                         'simcore::NoPhotoNuclearModel',
                         'SimCore_PhotoNuclearModels')
