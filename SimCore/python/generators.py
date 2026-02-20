"""Primary Generator templates for use throughout ldmx-sw"""

from LDMX.Framework import _register, field, parameter_set


class PrimaryGenerator:
    """Base for all primary generators for type checking"""

    pass


def primary_generator(class_name: str, module_name: str = "SimCore_Generators"):
    """Label a class as configuration for a primary generator

    Parameters
    ----------
    class_name : str
        Name of C++ class that this PrimaryGenerator should be
    module_name : str, optional
        Name of C++ library that this primary generator is compiled into

    Attributes
    ----------
    instance_name : str
        Unique name for this particular instance of a PrimaryGenerator
    beam_spot_smear : list of float, optional
        2 (x,y) or 3 (x,y,z) widths to smear primary vertices from this generator [mm].
        If set, this generator will handle its own beam spot smearing instead of using
        the global simulator beam_spot_smear setting.
    """
    return parameter_set(
        class_name=field(default=class_name, init=False),
        module_name=field(default=module_name, init=False),
        instance_name=class_name,
        beam_spot_smear=[20.0, 80.0, 0.0],
        post_init=lambda self: _register.library(self.module_name),
        required_base=PrimaryGenerator,
    )


@primary_generator("simcore::generators::ParticleGun")
class gun(PrimaryGenerator):
    """basic particle gun primary generator

    Attributes
    ----------
    time : float, optional
        Time to shoot from [ns]
    verbosity : int, optional
        Verbosity flag for this generator
    particle : str
        Geant4 particle name to shoot
    energy : float
        Energy of particle to shoot [GeV]
    position : list of float
        Position to shoot from [mm]
    direction : list of float
        Unit vector direction to shoot from

    Examples
    --------
        my_gun = gun(
            instance_name = '4gev-e-into-target',
            particle = 'e-',
            energy = 4.0,
            direction = [0., 0., 1.],
            position = [0., 0., 0.]
        )
    """

    time: float = 0.0
    verbosity: int = 0
    particle: str = ""
    energy: float = 0.0
    position: list[float] = []
    direction: list[float] = []
    __legacy__ = {"vertex": "position"}


@primary_generator("simcore::generators::MultiParticleGunPrimaryGenerator")
class multi(PrimaryGenerator):
    """multi particle gun primary generator

    Attributes
    ----------
    enable_poisson : bool, optional
        Poisson-distribute number of particles?
    vertex : list of float
        Position to shoot particle(s) from [mm]
    momentum : list of float
        3-momentum to give particle(s) in [MeV]
    n_particles : int, optional
        Number of particles to shoot (or average of Poisson distribution)
    pdg_id : int
        PDG ID of particle(s) to shoot
    """

    enable_poisson: bool = False
    vertex: list[float] = []
    momentum: list[float] = []
    n_particles: int = 1
    pdg_id: int = 0


@primary_generator("simcore::generators::LHEPrimaryGenerator")
class lhe(PrimaryGenerator):
    """LHE file primary generator

    Parameters
    ----------
    file_path : str
        path to LHE file containing the primary vertices
    vertex : list of float, optional
        Vertex position to shoot from [mm]. Defaults to [0.0, 0.0, 0.0]
    """

    file_path: str
    vertex: list[float] = [0.0, 0.0, 0.0]


@primary_generator("simcore::generators::GeneralParticleSource")
class gps(PrimaryGenerator):
    """general particle source

    The input initialization commands are run in the order that they are listed.

    Parameters
    ----------
    name : str
        name of new primary generator
    init_commands : list of strings
        List of Geant4 commands to initialize this GeneralParticleSource

    Returns
    -------
    simcfg.PrimaryGenerator
        configured to be a GeneralParticleSource with the passed initialization commands

    Examples
    --------
        my_gps = gps( 'my_gps' , [
            "/gps/particle e-",
            "/gps/pos/type Plane",
            "/gps/pos/shape Square",
            "/gps/pos/centre 0 0 0 mm",
            "/gps/pos/halfx 40 mm",
            "/gps/pos/halfy 80 mm",
            "/gps/ang/type cos",
            "/gps/ene/type Lin",
            "/gps/ene/min 3 GeV",
            "/gps/ene/max 4 GeV",
            "/gps/ene/gradient 1",
            "/gps/ene/intercept 1"
            ] )
    """

    init_commands: list[str]


@primary_generator("simcore::generators::GenieGenerator")
class genie(PrimaryGenerator):
    """Simple GENIE generator

    Attributes
    ----------
    energy : float
        Energy of particle to shoot [GeV]
    targets : list of int
        List of target nuclei PDG codes
    abundances : list of float
        List of abundances for target nuclei
    time : double
        Time to shoot from [ns]
    position : list of double
        Position of interaction from [mm]
    direction : list of double
        Unit vector direction of incoming electron
    tune: str
        Name of GENIE tune to use
    target_thickness : double
        Thickness of target [mm] to use for generation
    beam_size : list of double
        uniform beam size width to use

    Examples
    --------
        myGenie = genie( name='myGenie',
                         energy = 4.0,
                         targets = [ 1000220480 ],
                         abundances = [ 1.0 ],
                         time = 0.0,
                         position = [0.,0.,0.],
                         direction = [0.,0.,1.],
                         tune='G18_02a_00_000')
    """

    energy: float = 8.0
    targets: list[int] = []
    target_thickness: float = 0.3504
    abundances: list[float] = []
    time: float = 0.0
    position: list[float] = [0.0, 0.0, 0.0]
    beam_size: list[float] = [0.0, 0.0]
    direction: list[float] = [0.0, 0.0, 1.0]
    tune: str = "default"
    spline_file: str = ""
    message_threshold_file: str = "/usr/local/GENIE/Generator/config/Messenger.xml"


def _single_e_upstream_tagger(position, momentum, energy):
    """Internal helper function for creating electron beam guns upstream of tagger

    The guns position and momentum are determined by shooting a positron
    _backwards_ from the target without any detector components present. This
    uses Geant4 to calculate the trajectory within the magnetic field for us.
    Since the particle (if its high enough energy) will not interact with anything,
    you only need to generate a few events to get a good measurement.

    Parameters
    ----------
    position: list[float]
        end-point position of the positron shot backwards from target,
        is the start-point of an electron being shot into the target
    momentum: list[float]
        end-point momentum of the backwards positron **multiplied by -1**,
        defines the starting direction of an electron being shot into
        the target
    energy: float
        the energy of the electron in GeV, should be the same energy
        as the backwards positron used to deduce position and momentum

    Returns
    -------
    gun:
        configured instance of gun firing from far upstream of detector
    """

    import math

    momentum_mag = math.sqrt(sum(x * x for x in momentum))
    unit_direction = [x / momentum_mag for x in momentum]
    return gun(
        instance_name=f"single_{energy}gev_e_upstream_tagger",
        particle="e-",
        position=position,
        direction=unit_direction,
        energy=energy,
    )


def single_4gev_e_upstream_tagger():
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [-43.56748, 0.0, -883.0], [388.5554, 0.0, 3981.5967], 4.0
    )


def single_4gev_e_upstream_target():
    """Configure a particle gun to fire a 4 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by
    the field and arrive at the target at approximately [0, 0, 0] (assuming
    it's not smeared).

    Returns
    -------
    Instance of a particle gun configured to fire a single 4 Gev electron
    directly upstream of the target.
    """

    return gun(
        instance_name="single_4gev_e_upstream_target",
        particle="e-",
        position=[0.0, 0.0, -1.2],
        direction=[0.0, 0.0, 1],
        energy=4.0,
    )


def single_1pt2gev_e_upstream_tagger():
    """Configure a particle gun to fire a 8 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 8 GeV electron
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [-148.95303, 0.0, -883.0], [388.57147, 0.0, 1135.8867], 1.2
    )


def single_8gev_e_upstream_tagger():
    """Configure a particle gun to fire a 8 GeV electron upstream of the tagger tracker.

    The position and direction are set such that the electron will be bent by
    the field and arrive at the target at [0, 0, 0] if it isn't smeared and doesn't
    interact with any material. In reality, it will be smeared and it will interact
    with some material but we can dream.

    Returns
    -------
    Instance of a particle gun configured to fire a single 8 GeV electron
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [-21.745876, 0.0, -883.0], [388.55154, 0.0, 7991.0703], 8.0
    )


def single_e_beam_pipe(ene=8.0):
    """Configure a particle gun to fire an electron of settable energy
    upstream of the tagger tracker.

    The starting position here is well upstream of the analyzing magnet
    the position/angle of the gun is such that 8 gev electrons arrive
    at the target z=0 at xy=(0,0).  This generator is used to study
    off-energy beam electrons.

    Note that if an energy != 8gev, the trajectory will be different.
    And many electrons with energies sufficiently lower than 8GeV will just curve
    into the side of the magnet and not reach the target.

    Returns
    -------
    Instance of a particle gun configured to fire a single 8 GeV electron
    upstream of the entire detector apparatus.
    """
    return _single_e_upstream_tagger(
        [-299.2386690686212, 0.0, -6000.0],
        [434.59663056485, 0.0, 7988.698356992288],
        ene,
    )


def single_backwards_positron(energy: float):
    """A particle gun configured to shoot positrons backwards (i.e. upstream)
    from the target at the input energy.

    This generator is helpful for studying where electron guns of different energies should
    be started from if they should end up at the center of the target.

    Parameters
    ----------
    energy: float
        energy in GeV of the positron

    Returns
    -------
    gun:
        configured particle gun to shoot positrons backwards at the input energy
    """
    return gun(
        f"backwards-positron-{energy}GeV",
        particle="e+",
        position=[0.0, 0.0, 0.0],
        direction=[0.0, 0.0, -1.0],
        energy=energy,
    )
