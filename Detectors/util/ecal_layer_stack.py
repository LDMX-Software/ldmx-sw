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

    This class also has the ability to be multiplied by weights and added to other
    instances of this class, but that is purely implemented so that the `mixture`
    function below can be used.

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


    def __rmul__(self, weight):
        """weight this material by input fraction"""
        return PDGMaterial(
            density = weight*self.density,
            minimum_ionization = weight*self.minimum_ionization,
            radiation_length = weight*self.radiation_length,
            nuclear_interaction_length = weight*self.nuclear_interaction_length
        )


    def __add__(self, other):
        """add this material and another"""
        return PDGMaterial(
            density = self.density + other.density,
            minimum_ionization = self.minimum_ionization + other.minimum_ionization,
            radiation_length = self.radiation_length + other.radiation_length,
            nuclear_interaction_length = self.nuclear_interaction_length + other.nuclear_interaction_length
        )

    @classmethod
    def zero(cls):
        """we need a zero-object to start summing from in the mixture function"""
        return cls(0,0,0,0)


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
    # this carbon is 6 C carbon (graphite)
    C = PDGMaterial(
        density = 2.210,
        minimum_ionization = 1.742,
        nuclear_interaction_length = 85.8,
        radiation_length = 42.70
    ),
    polycarbonate = PDGMaterial(
        density = 1.200,
        minimum_ionization = 1.886,
        nuclear_interaction_length = 83.6,
        radiation_length = 41.50
    )
)


def pdg_material(**kwargs):
    """Estimate material properties by doing a weighted sum of its components
    from the PDG material table copied from online.

    The input key-word arguments specify the material (key) and its fraction (value).
    For example, the following would produce a material which is 50% copper and 50% silicon.

        pdg_material(Cu = 0.5, Si = 0.5)

    """
    
    weight_sum = sum(weight for weight in kwargs.values())
    if weight_sum != 1.0:
        raise ValueError(f"Sum of weights provided ({weight_sum}) does not equal 1.0: {kwargs}")
    return sum((weight*__materials__[material] for material, weight in kwargs.items()), PDGMaterial.zero())


def print_gdml_list(**kwargs) :
    for name, l in kwargs.items() :
        newline_list = f'{l[0]:.1f}\n' + '\n'.join([f'                {v:.1f}' for v in l[1:]])
        print(f'<matrix name="{name}"')
        print( '        coldim="1"')
        print(f'        values="{newline_list}"/>')

def enumerate_absorber_dz(sections) :
    cooling_tungsten_dz = []
    front_tungsten_dz = []
    bilayer_absorber_cumulative = [0.]
    for name, num_bilayers, front, cooling in sections :
        for bilayer in range(num_bilayers) :
            cooling_tungsten_dz.append(cooling)
            front_tungsten_dz.append(front)
            bilayer_absorber_cumulative.append(bilayer_absorber_cumulative[-1] + 2*cooling + front)
    return cooling_tungsten_dz, front_tungsten_dz, bilayer_absorber_cumulative

class Layer :
    """class representing a single layer of a single material

    In order to align with Geant4, we use the following units
    - Energy: MeV
    - Distance: mm

    Class Attributes
    ----------------
    We keep a few reference tables stored within the class so that all layers
    can access the properties of their materials. The materials that are single elements,
    we obtain their properties from [Atomic and Nuclear Properties](https://pdg.lbl.gov/2023/AtomicNuclearProperties/index.html)
    from the PDG. The composite materials are also found from this link, but we may
    be forced to use a similar material rather than one that perfectly matches our
    GDML definiton. The unit-conversion calculations is left here for transparency.

    Here   | PDG
    -------|-----
    Al     | Al
    Air    | Mixtures -> Air (dry, 1 atm)
    PCB    | weighted mix of elements
    Si     | Si
    W      | W
    Carbon | C -> 6 C carbon (graphite)
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
        PCB = pdg_material(
            Cu = 0.5,
            O = 0.22990022990023,
            Na = 0.0482205482205484,
            Si = 0.168276668276668,
            Ca = 0.0536025536025536
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

    SensDetThickness = 0.3

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

    def enumerate_full_stack(sections) :
        layers = []
        for name, num_bilayers, front, cooling in sections :
            for bilayer in range(num_bilayers) :
                layers.append(Layer.air(0.5)) #Front_Tolerance
                if front > 0 :
                    layers.append(Layer.tungsten(front))
                    layers.append(Layer.air(0.5))
                if front == 0:
                    layers.append(Layer.air(0.15)) # correction
                layers.append(Layer.pcb()) # PCB_dz
                layers.append(Layer.air(3.5)) # PCB_Motherboard_Gap
                layers.append(Layer.pcb()) # PCB_dz
                layers.append(Layer.glue(0.1)) # Glue_dz
                layers.append(Layer.silicon()) # Si_dz
                layers.append(Layer.glue(0.2)) # GlueThick_dz
                layers.append(Layer.carbon(0.79)) # CarbonBasePlate_dz
                if cooling == 0 :
                    layers.append(Layer.air(0.5)) # preshower_extra_air
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.carbon(5.7)) # CarbonCoolingPlane_dz
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.carbon(0.79)) # CarbonBasePlate_dz
                layers.append(Layer.glue(0.2)) # GlueThick_dz
                layers.append(Layer.silicon()) # Si_dz
                layers.append(Layer.glue(0.1)) # Glue_dz
                layers.append(Layer.pcb()) # PCB_dz
                layers.append(Layer.air(3.5)) # PCB_Motherboard_Gap
                if cooling == 0 : # sampling_section_offset
                    layers.append(Layer.air(0.5))
                    layers.append(Layer.air(0.5))
                layers.append(Layer.pcb()) # PCB_dz
    
        return layers

def average(raw_weights) :
    averaged = []
    # first to (n-1)th layers where n is the total number of layers
    for i_layer in range(0,len(raw_weights)-1) :
        averaged.append(0.5*(raw_weights[i_layer] + raw_weights[i_layer+1]))
    averaged.append(raw_weights[-1])
    return averaged

def materials_between_sensdet(layer_stack) :
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
    # Does not include sensitive detector layers
    dE_between_sensdet = [ ]
    X0_between_sensdet = [ ]
    L_between_sensdet  = [ ]
    # Does include sensitive detector layers
    Zpos_layer = [ ]
    for section in layers_partitioned_by_sensdet :
        dE_section, X0_section, L_section, Zdepth_section = 0., 0., 0., 0.
        for l in section :
            dE_section += l.thickness * l.dEdx
            X0_section += l.thickness / l.x0
            L_section  += l.thickness / l.nuclen
            Zdepth_section += l.thickness
        dE_between_sensdet.append(dE_section)
        X0_between_sensdet.append(X0_section)
        L_between_sensdet.append(L_section)
    
        last_layer_pos = 0.0
        if len(Zpos_layer) > 0:
            last_layer_pos = Zpos_layer[-1] + Layer.SensDetThickness
        Zpos_layer.append( last_layer_pos + Zdepth_section )
    #endfor - sections

    dE_between_sensdet = average(dE_between_sensdet)
    X0_between_sensdet = average(X0_between_sensdet)
    L_between_sensdet  = average(L_between_sensdet)

    return dE_between_sensdet, X0_between_sensdet, L_between_sensdet, Zpos_layer


def print_weights(dE_between_sensdet, X0_between_sensdet, L_between_sensdet, Zpos_layer, 
            output = sys.stdout) :
    output.write('{0:>5s} {1:>7s} {2:>6s} {3:>6s} {4:>6s}\n'.format('Layer', 'dE', 'X0', 'Lambda', 'Zpos'))
    for layer in range(len(dE_between_sensdet)-1):
        output.write('{0:5d} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
            layer+1, dE_between_sensdet[layer], X0_between_sensdet[layer], L_between_sensdet[layer], Zpos_layer[layer]))
    #endfor - layers
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Sum', sum(dE_between_sensdet[:-1]), sum(X0_between_sensdet[:-1]), sum(L_between_sensdet[:-1]), Zpos_layer[-1] ))
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Back', dE_between_sensdet[-1], X0_between_sensdet[-1], L_between_sensdet[-1], Zpos_layer[-1]))
    output.flush()


def command(func):
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
    # section, bilayers, front, cooling
    absorber_sections = [
            ('a',1,1,1),
            ('b',1,2,1.5),
            ('c',9,3.5,1.8),
            ('d',5,7,3.5)
            ]

    ct, ft, bac = enumerate_absorber_dz(absorber_sections)
    print_gdml_list(cooling_tungsten_dz = ct,
                    front_tungsten_dz = ft,
                    bilayer_absorber_cumulative = bac)

    layers = Layer.enumerate_full_stack([('ps',1,0,0)]+absorber_sections)
    mbs = materials_between_sensdet(layers)
    weights = calc_weights(mbs)
    print_weights(*weights)

#    for l in layers :
#        print(l)

@command
def minildmx():
    # section, bilayers, front, cooling
    print('            |     Depth     |')
    print('N Bi-Layers | X0    | mm    |')
    for n in range(1,4):
        layers = Layer.enumerate_full_stack([('a',n,0,0)])
        print('{n:>11} | {x0:<5.3g} | {z:<5.3g} |'.format(
            n = n,
            x0 = sum(layer.thickness / layer.x0 for layer in layers),
            z = sum(layer.thickness for layer in layers)
        ))


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('command', choices = list(command.__list__))
    args = parser.parse_args()

    command.__list__[args.command]()


if __name__ == '__main__' :
    main()
