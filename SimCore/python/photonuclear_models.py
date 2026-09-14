"""Configuration classes for default photonuclear models"""

from LDMX.Framework import _register, field, parameter_set


class PhotoNuclearModel:
    pass


def photo_nuclear_model(
    class_name: str, module_name: str = "SimCore_PhotoNuclearModels"
):
    """Configuration for a photonuclear model that we want to load

    Parameters
    ----------
    class_name : str
        Name of C++ class that this PhotoNuclear model should be
    module_name : str
        Name of C++ library that this PhotoNuclear model is compiled into
    """
    return parameter_set(
        class_name=field(default=class_name, init=False),
        instance_name=class_name,
        module_name=field(default=module_name, init=False),
        post_init=lambda self: _register.library(self.module_name),
        required_base=PhotoNuclearModel,
    )


@photo_nuclear_model("simcore::BertiniModel")
class BertiniModel(PhotoNuclearModel):
    """The default model for photonuclear interactions.

    Keeps the default Bertini model from Geant4.
    """

    pass


@photo_nuclear_model("simcore::BertiniNothingHardModel")
class BertiniNothingHardModel(PhotoNuclearModel):
    """A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

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

    count_light_ions: bool = True
    hard_particle_threshold: float = 200.0
    zmin: int = 74
    emin: float = 2500.0


@photo_nuclear_model("simcore::BertiniSingleNeutronModel")
class BertiniSingleNeutronModel(PhotoNuclearModel):
    """A photonuclear model producing only topologies where only one neutron has
    kinetic energy above a particular threshold.

    Uses the default Bertini model from Geant4.

    """

    hard_particle_threshold: float = 200.0
    zmin: int = 0
    emin: float = 2500.0
    count_light_ions: bool = True


@photo_nuclear_model("simcore::BertiniAtLeastNProductsModel")
class BertiniAtLeastNProductsModel(PhotoNuclearModel):
    """A photonuclear model producing only topologies with no particles above a
    certain threshold.

    Uses the default Bertini model from Geant4.

    """

    min_products: int
    pdg_ids: list[int] = []
    hard_particle_threshold: float = 200.0
    zmin: int = 0
    emin: float = 2500.0

    def kaon(self=1, hard_particle_threshold=200.0):
        # Note: By default, this is requiring at least 1 kaon with at least 200
        # MeV. You may want a different energy threshold depending on your needs.
        return BertiniAtLeastNProductsModel(
            min_products=self,
            instance_name=f"{self}_kaon_model",
            hard_particle_threshold=hard_particle_threshold,
            pdg_ids=[
                130,  # K_L^0
                310,  # K_S^0
                311,  # K^0
                321,  # K^+
                -321,  # K^-
            ],
        )


@photo_nuclear_model("simcore::BertiniExactlyNProductsModel")
class BertiniExactlyNProductsModel(PhotoNuclearModel):
    """A photonuclear model producing only topologies with a define number
    of particles above a certain threshold.

    Uses the default Bertini model from Geant4.
    """

    n_products: int
    pdg_ids: list[int]
    hard_particle_threshold: float = 200.0
    zmin: int = 0
    emin: float = 2500.0
    check_allmatch: bool = False

    def kaon(self=2, hard_particle_threshold=200.0):
        # This is requiring exactly 2 kaons with at least 200 MeV.
        return BertiniExactlyNProductsModel(
            instance_name=f"{self}_kaon_model",
            n_products=self,
            pdg_ids=[
                130,  # K_L^0
                310,  # K_S^0
                311,  # K^0
                321,  # K^+
                -321,  # K^-
            ],
            hard_particle_threshold=hard_particle_threshold,
        )

    def neutron(self=1, hard_particle_threshold=200.0):
        return BertiniExactlyNProductsModel(
            instance_name=f"{self}_neutron_model",
            n_products=self,
            hard_particle_threshold=hard_particle_threshold,
            pdg_ids=[2212],
        )


@photo_nuclear_model("simcore::NoPhotoNuclearModel")
class NoPhotoNuclearModel(PhotoNuclearModel):
    """A PhotoNuclear model that disables the photonuclear process entirely.

    Make sure that no biasing operators for photonuclear reactions are enabled
    when using this model.
    """

    pass


@photo_nuclear_model("simcore::bertini::BertiniWithHistoryModel", "SimCore_Bertini")
class BertiniWithHistoryModel(PhotoNuclearModel):
    """Bertini cascade with internal history recording.

    Uses the standard Bertini cascade but captures the step-by-step cascade
    history for each photonuclear interaction. The history includes all particles,
    parent-daughter relationships, momenta, positions, and quasi-deuteron target
    types (pp, pn, nn). Output is stored as "PhotonuclearCascadeHistories".

    By default, only cascades initiated by photons above 5 GeV are recorded
    (matching the typical ECal PN bias threshold).

    Examples
    --------
        model = BertiniWithHistoryModel()
        model.energy_threshold = 2500.0  # record above 2.5 GeV
    """

    max_energy: float = 15000.0
    energy_threshold: float = 5000.0
