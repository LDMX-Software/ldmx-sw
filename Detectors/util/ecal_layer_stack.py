"""Calculate the depth parameters for the ECal layers

This is a stand-alone python script meant to help make these calculations easer.
It is kept within ldmx-sw since both the GDML and the reconstruction layer weights
have been updated using the output of this script.
"""

import sys
from dataclasses import dataclass, asdict
import json


@dataclass
class PDGMaterial:
    """a material copied down from the PDG's Atomic and Nuclear Properties site

    Attributes
    ----------
    density: float
        density of material in g / cm^3
    minimum_ionization: float
        MeV * cm^2 / g
    radiation_length: float
        g / cm^2
    nuclear_interaction_length: float
        g / cm^2
    """

    density: float
    minimum_ionization: float
    radiation_length: float
    nuclear_interaction_length: float


    def minimum_ionization_MeV_mm(self):
        """need to resolve the minimum ionization into MeV/mm units"""
        return (self.minimum_ionization * self.density) / 10


    def radiation_length_mm(self):
        """resolve the radiation length into mm units"""
        return (self.radiation_length / self.density) * 10


    def nuclear_interaction_length_mm(self):
        """resolve the nuclear interaction length into mm"""
        return (self.nuclear_interaction_length / self.density) * 10


class PDGMaterialEncoder(json.JSONEncoder):
    """Encode a PDGMaterial into JSON

    This just returns a dictionary of the dataclass members
    when encoding to JSON so that we can pretty-print the material
    table with indenting for easier readability.
    For example

        print(json.dumps(Layer.materials, cls=PDGMaterialEncoder, indent=2))

    """

    def default(self, o):
        if isinstance(o, PDGMaterial):
            return asdict(o)
        return super().default(o)


# This dictionary holds materials that are copied down from the PDG site
__materials__ = dict(
    Al = PDGMaterial(
        density = 2.699,
        minimum_ionization = 1.615,
        nuclear_interaction_length = 107.2,
        radiation_length = 24.01
    ),
    Air = PDGMaterial(
        density = 1.205e-3,
        minimum_ionization = 1.815,
        nuclear_interaction_length = 90.1,
        radiation_length = 36.62
    ),
    SuperDenseAir = PDGMaterial(
        density = 1.205,
        minimum_ionization = 1.815,
        nuclear_interaction_length = 90.1,
        radiation_length = 36.62
    ),
    Cu = PDGMaterial(
        density = 8.960,
        minimum_ionization = 1.403,
        nuclear_interaction_length = 137.3,
        radiation_length = 12.86
    ),
    # this oxygen is oxygen gas
    O = PDGMaterial(
        density = 1.332e-3,
        minimum_ionization = 1.801,
        nuclear_interaction_length = 90.2,
        radiation_length = 34.24
    ),
    Na = PDGMaterial(
        density = 0.9710,
        minimum_ionization = 1.639,
        nuclear_interaction_length = 102.6,
        radiation_length = 27.74
    ),
    Si = PDGMaterial(
        density = 2.329,
        minimum_ionization = 1.664,
        nuclear_interaction_length = 108.4,
        radiation_length = 21.82
    ),
    Ca = PDGMaterial(
        density = 1.550,
        minimum_ionization = 1.655,
        nuclear_interaction_length = 119.8,
        radiation_length = 16.14
    ),
    W = PDGMaterial(
        density = 19.30,
        minimum_ionization = 1.145,
        nuclear_interaction_length = 191.9,
        radiation_length = 6.76
    ),
    # this carbon is 6 C carbon (amorphous)
    # with a lower density to represent carbon fiber
    C = PDGMaterial(
        density = 1.800,
        minimum_ionization = 1.749,
        nuclear_interaction_length = 85.8,
        radiation_length = 42.70
    ),
    # FR4 is a fire-retardant version of G10 with 7-8% bromide
    G10 = PDGMaterial(
        density = 1.800,
        minimum_ionization = 1.762,
        nuclear_interaction_length = 78.4,
        radiation_length = 32.17
    ),
    polycarbonate = PDGMaterial(
        density = 1.200,
        minimum_ionization = 1.886,
        nuclear_interaction_length = 83.6,
        radiation_length = 41.50
    )
)


def pdg_material(*, scale_weights=False, **kwargs):
    """Estimate material properties by doing a weighted sum of its components
    from the PDG material table copied from online.

    The input key-word arguments specify the material (key) and its fraction (value).
    For example, the following would produce a material which is 50% copper and 50% silicon.

        pdg_material(Cu = 0.5, Si = 0.5)

    If the input weights do not sum to 1.0, then a ValueError is raised (if scale_weights is False)
    or the weights are all divided by their sum.
    """
    
    weight_sum = sum(weight for weight in kwargs.values())
    if weight_sum != 1.0:
        if not scale_weights:
            raise ValueError(f"Sum of weights provided ({weight_sum}) does not equal 1.0: {kwargs}")

        # divide all weights by the weight sum so it equals 1
        for key in kwargs:
            kwargs[key] /= weight_sum

    return PDGMaterial(
        density = sum(weight*__materials__[material].density for material, weight in kwargs.items()),
        minimum_ionization = sum(
            weight*__materials__[material].minimum_ionization
            for material, weight in kwargs.items()
        ),
        radiation_length = 1/(sum(
            weight/__materials__[material].radiation_length
            for material, weight in kwargs.items())
        ),
        nuclear_interaction_length = 1/(sum(
            weight/__materials__[material].nuclear_interaction_length
            for material, weight in kwargs.items())
        )
    )


class Layer :
    """class representing a single layer of a single material

    In order to align with Geant4, we use the following units
    - Energy: MeV
    - Distance: mm

    Class Attributes
    ----------------
    We keep a few reference tables stored within the class so that all layers
    can access the properties of their materials.
    The materials that are copied from the PDG
    [Atomic and Nuclear Properties](https://pdg.lbl.gov/2023/AtomicNuclearProperties/index.html)
    are kept as PDGMaterial in the __materials__ dictionary.
    Some composite materials are also found in the PDG, but we have to estimate the PCB
    material properties ourselves.
    The unit-conversion calculations are left in PDGMaterial for transparency.

    Natalia found [this study of the ALICE TRD](https://www-physics.lbl.gov/~gilg/PixelUpgradeMechanicsCooling/Material/Radiationlength.pdf)
    which is a helpful source of comparison to make sure we aren't wildly off.

    Here   | PDG
    -------|-----
    Al     | Al
    Air    | Mixtures -> Air (dry, 1 atm)
    PCB    | weighted mix of materials
    Si     | Si
    W      | W
    Carbon | C -> 6 C carbon (amorphous) with lower density to represent carbon fiber
    Glue   | Polymers -> polycarbonate (OC6H4C(CH3)2C6H4OCO)n

    PCB in the GDML is 50% Cu, 23% O, 4.8% Na, 17% Si, 5.2% Ca,
    and the PDG does not have any mixtures in the drop down menu
    that have Copper in them so I have to spin my own.

    Glue in the GDML is a polycarbon is 85% C, 4% H, and 11% O,
    the polycarbonate in the PDG is 75% C, 5% H and 20% O which
    I deemed close enough.

    dEdx : dict[str, float]
        material name to average energy loss per unit distance (MeV/mm) of a MIP.
        If listed in the PDG, they are the "Minimum ionization" line.
        The calculation is (dEdx [MeV cm^2/g] * density [g/cm^3]) / 10 [mm/cm]

    X0 : dict[str, float]
        material name to radiation length (mm)
        These values are taken from the "Radiation length" line of the PDG if possible.
        The calculation is (X0 [g/cm^2] / density [g/cm^3]) * 10 [mm/cm].

    nuclen : dict[str, float]
        material name to nuclear interaction length (mm)
        These values are taken from the "Nuclear interaction length" line of the PDG if possible.
        The calculation is (nuclen [g/cm^2] / density [g/cm^3]) * 10 [mm/cm].
    """

    materials = dict(
        Al = pdg_material(Al = 1.0),
        Air = pdg_material(Air = 1.0),
#        # using atomic fraction
#        PCB = pdg_material(
#            Cu = 0.5,
#            G10 = 0.5,
#            scale_weights = True
#        ),
#        # using depth/thickness fraction from ALICE TRD
#        PCB = pdg_material(
#            Cu = 0.025,
#            G10 = 0.38,
#            scale_weights = True
#        ),
        # using depth/thickness fraction from pcb-layers.nbt
        PCB = pdg_material(
            Cu = 0.238,
            G10 = 1.422,
            scale_weights=True
        ),
        Si = pdg_material(Si = 1.0),
        W = pdg_material(W = 1.0),
        Carbon = pdg_material(C = 1.0),
        Glue = pdg_material(polycarbonate = 1.0)
    )

    dEdx = {
        name : material.minimum_ionization_MeV_mm()
        for name, material in materials.items()
    }

    X0 = {
        name: material.radiation_length_mm()
        for name, material in materials.items()
    }
    
    nuclen = {
        name: material.nuclear_interaction_length_mm()
        for name, material in materials.items()
    }

    # 300 um for v14 and 400 um for v15
    SensDetThickness = 0.4

    def __init__(self, name, thickness, sensitive = False) :
        self.name = name
        self.thickness = thickness
        self.sensitive = sensitive
        self.nuclen = Layer.nuclen[self.name]
        self.x0 = Layer.X0[self.name]
        self.dEdx = Layer.dEdx[self.name]

    def __str__(self) :
        return f'{self.thickness:.2f} mm {self.name}'

    def air(t) :
        return Layer('Air',t)

    def tungsten(t) :
        return Layer('W', t)

    def pcb() :
        return Layer('PCB',1.666)

    def glue(t) :
        return Layer('Glue', t)

    def silicon() :
        return Layer('Si',Layer.SensDetThickness,sensitive=True)

    def carbon(t) :
        return Layer('Carbon',t)


@dataclass
class BiLayerSandwich:
    """A "bilayer" in the ECal consists of two (hence 'bi-') sensitive
    silicon layers mounted onto a central carbon cooling plane.

    There are additional layers of air, PCBs, and potentially absorber.
    Since the thickness of the absorber varies, its thickness is stored
    as members in this class.

    Attributes
    ----------
    cooling: float
        thickness in mm of absorber mounted between carbon cooling plane and
        hexamodules
    front: float
        thickness in mm of absorber put "in front" (upstream/lower z) of the
        bilayer so that there is absorber between adjacent bilayers

    For many bilayers, the "front" absorber is twice the
    thickness of the "cooling" so that approximately the same absorber is
    between the sensitive hexamodules. In the real detector, the "front" absorber
    will actually be just two copies of the "cooling" absorber plates in
    this case.

    The full implementation is given below, but a diagram might be helpful.

       ↓ beam ↓ going down

      | front absorber (W)            |
      | Readout Motherboard (PCB)     |
      | Air                           |
      | Hexamodule (Si, Glue, PCB, C) |
      | Cooling Absorber (W)          |
      | Carbon Cooling Plane (Carbon) |
      | Cooling Absorber (W)          |
      | Hexamodule (Si, Glue, PCB, C) |
      | Air                           |
      | Readout Motherboard (PCB)     |

    """

    front: float = 0.0
    cooling: float = 0.0

    def material_stack(self):
        layers = []
        layers.append(Layer.air(0.5)) #Front_Tolerance
        if self.front > 0 :
            layers.append(Layer.tungsten(self.front))
            layers.append(Layer.air(0.5))
        if self.front == 0:
            layers.append(Layer.air(0.15)) # correction
        layers.append(Layer.pcb()) # PCB_dz
        layers.append(Layer.air(3.5)) # PCB_Motherboard_Gap
        layers.append(Layer.pcb()) # PCB_dz
        layers.append(Layer.glue(0.1)) # Glue_dz
        layers.append(Layer.silicon()) # Si_dz
        layers.append(Layer.glue(0.2)) # GlueThick_dz
        layers.append(Layer.carbon(0.79)) # CarbonBasePlate_dz
        if self.cooling == 0 :
            layers.append(Layer.air(0.5)) # preshower_extra_air
        if self.cooling > 0 :
            layers.append(Layer.tungsten(self.cooling))
        layers.append(Layer.carbon(5.7)) # CarbonCoolingPlane_dz
        if self.cooling > 0 :
            layers.append(Layer.tungsten(self.cooling))
        layers.append(Layer.carbon(0.79)) # CarbonBasePlate_dz
        layers.append(Layer.glue(0.2)) # GlueThick_dz
        layers.append(Layer.silicon()) # Si_dz
        layers.append(Layer.glue(0.1)) # Glue_dz
        layers.append(Layer.pcb()) # PCB_dz
        layers.append(Layer.air(3.5)) # PCB_Motherboard_Gap
        if self.cooling == 0 : # sampling_section_offset
            layers.append(Layer.air(0.5))
            layers.append(Layer.air(0.5))
        layers.append(Layer.pcb()) # PCB_dz
        return layers


def print_gdml_list(**kwargs) :
    """print a list (or lists) into GDML format to stdout

    The key-word arguments are 'name = l' where 'name' is the name
    of the list as it should appear in the GDML and 'l' is the python list
    that will be injected into the GDML.
    """

    for name, l in kwargs.items() :
        newline_list = f'{l[0]:.1f}\n' + '\n'.join([f'                {v:.1f}' for v in l[1:]])
        print(f'<matrix name="{name}"')
        print( '        coldim="1"')
        print(f'        values="{newline_list}"/>')


def enumerate_absorber_dz(bilayers) :
    """go through layers and keep track of the longitudinal depth (dz) of the absorber"""

    cooling_tungsten_dz = []
    front_tungsten_dz = []
    bilayer_absorber_cumulative = [0.]
    for bilayer in bilayers:
        cooling_tungsten_dz.append(bilayer.cooling)
        front_tungsten_dz.append(bilayer.front)
        bilayer_absorber_cumulative.append(
            bilayer_absorber_cumulative[-1] + 2*bilayer.cooling + bilayer.front
        )
    return cooling_tungsten_dz, front_tungsten_dz, bilayer_absorber_cumulative


def average(raw_weights) :
    """rolling average of the weights between adjacent layers"""
    averaged = []
    # first to (n-1)th layers where n is the total number of layers
    for i_layer in range(0,len(raw_weights)-1) :
        averaged.append(0.5*(raw_weights[i_layer] + raw_weights[i_layer+1]))
    averaged.append(raw_weights[-1])
    return averaged


def materials_between_sensdet(layer_stack) :
    """partition the input layer stack by the sensitive layers

    The returned object is a list of lists such that the sub-lists
    are the layer stacks between adjacent sensitive layers.
    The sensitive layers are left out of these sub-lists.
    """

    mbs = []
    current = []
    for layer in layer_stack :
        if layer.sensitive :
            mbs.append(current)
            current = []
        else :
            current.append(layer)
    if len(current) > 0 :
        mbs.append(current)
    return mbs


def calc_weights(layers_partitioned_by_sensdet) :
    """calculate different material properties for the layer stacks between sensitive layers

    These material properties do not include the sensitive layers themselves since
    the partitioning drops them out (see materials_between_sensdet).
    The returned lists are in the same order as the list of lists provided so they
    can be mapped onto the sensitive layers that follow them.
    We keep track of
    - dE: average energy loss
    - X0: radiation length
    - L: nuclear interaction length
    - Z: longitudinal depth

    The dE, X0, and L arrays are then updated with a the rolling average to "smooth out"
    the weights. Without this rolling average, the weights "oscillate" up and down
    due to design of the bilayer.
    """

    # Does not include sensitive detector layers
    dE_between_sensdet = [ ]
    X0_between_sensdet = [ ]
    L_between_sensdet  = [ ]
    # Does include sensitive detector layers
    Zpos_layer = [ ]
    for section in layers_partitioned_by_sensdet :
        dE_between_sensdet.append(sum(l.thickness * l.dEdx for l in section))
        X0_between_sensdet.append(sum(l.thickness / l.x0 for l in section))
        L_between_sensdet.append(sum(l.thickness / l.nuclen for l in section))
        last_layer_pos = 0.0
        if len(Zpos_layer) > 0:
            last_layer_pos = Zpos_layer[-1] + Layer.SensDetThickness
        Zpos_layer.append(last_layer_pos + sum(l.thickness for l in section))
    #endfor - sections

    dE_between_sensdet = average(dE_between_sensdet)
    X0_between_sensdet = average(X0_between_sensdet)
    L_between_sensdet  = average(L_between_sensdet)

    return dE_between_sensdet, X0_between_sensdet, L_between_sensdet, Zpos_layer


def print_weights(
    dE_between_sensdet,
    X0_between_sensdet,
    L_between_sensdet,
    Zpos_layer, 
    output = sys.stdout
):
    """print the weights in a nice-ly formatted table

    The order of inputs to this function is the same as the outputs of the calc_weights
    function so they can be called directly following each other.
    """
    output.write(
        '{0:>5s} {1:>7s} {2:>6s} {3:>6s} {4:>6s}\n'.format(
            'Layer', 'dE', 'X0', 'Lambda', 'Zpos'
        )
    )
    output.write('-----------------------------------\n')
    for layer in range(len(dE_between_sensdet)-1):
        output.write('{0:5d} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
            layer+1, dE_between_sensdet[layer], X0_between_sensdet[layer], L_between_sensdet[layer], Zpos_layer[layer]))
    #endfor - layers
    output.write('-----------------------------------\n')
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Sum', sum(dE_between_sensdet[:-1]), sum(X0_between_sensdet[:-1]), sum(L_between_sensdet[:-1]), Zpos_layer[-1] ))
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Back', dE_between_sensdet[-1], X0_between_sensdet[-1], L_between_sensdet[-1], Zpos_layer[-1]))
    output.flush()


def command(func):
    """register a function as a command line command"""
    command.__list__[func.__name__] = func
    return func


command.__list__ = {}


@command
def print_layer_materials():
    print(json.dumps({
        name : {
            attr: getattr(material, attr)()
            for attr in [
                'minimum_ionization_MeV_mm',
                'radiation_length_mm',
                'nuclear_interaction_length_mm'
            ]
        }
        for name, material in Layer.materials.items()
    }, indent=2))


@command
def ldmx_ecal_v14():
    """full LDMX Ecal v14 geometry"""

    bilayers = (
        [BiLayerSandwich(front = 0.0, cooling = 0.0)] # absorber-less Pre-Shower
        +[BiLayerSandwich(front = 1.0, cooling = 1.0)] # Section A
        +[BiLayerSandwich(front = 2.0, cooling = 1.5)] # Section B
        +9*[BiLayerSandwich(front = 3.5, cooling = 1.8)] # Section C
        +5*[BiLayerSandwich(front = 7.0, cooling = 3.5)] # Section D
    )

    # the way I designed the GDML did not include the Pre-Shower bilayer
    # in these lists because the Pre-Shower bilayer doesn't have any absorber
    ct, ft, bac = enumerate_absorber_dz(bilayers[1:])
    print_gdml_list(cooling_tungsten_dz = ct,
                    front_tungsten_dz = ft,
                    bilayer_absorber_cumulative = bac)

    # adding two lists together just appends them, so I "sum" all the
    # bilayer material stacks into a single materal stack for the entire detector
    layers = sum((bilayer.material_stack() for bilayer in bilayers), [])
    # partition the material stack into groups separated by sensitive silicon
    mbs = materials_between_sensdet(layers)
    weights = calc_weights(mbs)
    print_weights(*weights)

@command
def ldmx_ecal_v15():
    """full LDMX Ecal v15 geometry"""

    bilayers = (
        [BiLayerSandwich(front = 0.0, cooling = 0.0)] # absorber-less Pre-Shower
        +[BiLayerSandwich(front = 1.0, cooling = 1.0)] # Section A
        +[BiLayerSandwich(front = 2.0, cooling = 1.5)] # Section B
        +9*[BiLayerSandwich(front = 3.5, cooling = 1.8)] # Section C
        +5*[BiLayerSandwich(front = 7.0, cooling = 3.5)] # Section D
    )

    # the way I designed the GDML did not include the Pre-Shower bilayer
    # in these lists because the Pre-Shower bilayer doesn't have any absorber
    ct, ft, bac = enumerate_absorber_dz(bilayers[1:])
    print_gdml_list(cooling_tungsten_dz = ct,
                    front_tungsten_dz = ft,
                    bilayer_absorber_cumulative = bac)

    # adding two lists together just appends them, so I "sum" all the
    # bilayer material stacks into a single materal stack for the entire detector
    layers = sum((bilayer.material_stack() for bilayer in bilayers), [])
    # partition the material stack into groups separated by sensitive silicon
    mbs = materials_between_sensdet(layers)
    weights = calc_weights(mbs)
    print_weights(*weights)

@command
def minildmx():
    print('            |     Depth     |')
    print('N Bi-Layers | X0    | mm    |')
    for n in range(1,4):
        layers = n*BiLayerSandwich(front=0, cooling=0).material_stack()
        print('{n:>11} | {x0:<5.3g} | {z:<5.3g} |'.format(
            n = n,
            x0 = sum(layer.thickness / layer.x0 for layer in layers),
            z = sum(layer.thickness for layer in layers)
        ))


@command
def bilayer_spec():
    layers = BiLayerSandwich(front=0, cooling=0).material_stack()
    totals_by_material = {}

    print('Full Layer Stack')
    print('Material, Depth / mm')
    for layer in layers:
        print(layer.name, layer.thickness, sep=', ')
        if layer.name not in totals_by_material:
            totals_by_material[layer.name] = 0.0
        totals_by_material[layer.name] += layer.thickness

    print()
    print('Total Depths')
    print('Material, Depth / mm')
    for material, depth in totals_by_material.items():
        print(material, depth, sep=', ')


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices = list(command.__list__))
    args = parser.parse_args()

    command.__list__[args.command]()


if __name__ == '__main__' :
    main()
