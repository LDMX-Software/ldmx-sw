"""Calculate the depth parameters for the ECal absorber layers"""


# dictionary of properties for material layers
# Units: MeV/mm
# Calculations are (dEdx [MeV cm^2/g] * density [g/cm^3]) / 10 [mm/cm]
dEdx = {
    'Al'     : (2.699 * 1.615) / 10,
    'Air'    : (1.815 * 1.205) / 10,
    'PCB'    : (1.815 * 1.205) / 10,
    'Si'     : (2.329 * 1.664) / 10,
    'W'      : (19.3  * 1.145) / 10,
    'Carbon' : (1.742 * 2.210) / 10,
    'Glue'   : (1.815 * 1.205) / 10
}

# Units: mm
# Calculations are (X0 [g/cm^2] / density [g/cm^3]) * 10 [mm/cm]
X0 = {
    'Al'     : ( 24.01 / 2.699 ) * 10,
    'Air'    : ( 36.62 / 1.205 ) * 10,
    'PCB'    : ( 36.62 / 1.205 ) * 10,
    'Si'     : ( 21.82 / 2.329 ) * 10,
    'W'      : ( 6.76  / 19.3  ) * 10,
    'Carbon' : ( 42.70 / 2.210 ) * 10,
    'Glue'   : ( 36.62 / 1.205 ) * 10
}

# Units: mm
# Calculations are (NucLen [g/cm^2] / density [g/cm^3]) * 10 [mm/cm]
nuclen = {
    'Al'     : ( 107.2 / 2.699 ) * 10,
    'Air'    : ( 90.1  / 1.205 ) * 10,
    'PCB'    : ( 90.1  / 1.205 ) * 10,
    'Si'     : ( 108.4 / 2.329 ) * 10,
    'W'      : ( 191.9 / 19.3  ) * 10,
    'Carbon' : ( 85.8  / 2.210 ) * 10,
    'Glue'   : ( 90.1  / 1.205 ) * 10
}


def print_gdml_list(**kwargs) :
    for name, l in kwargs.items() :
        newline_list = f'{l[0]:.1f}\n'
                     + '\n'.join([f'                {v:.1f}' for v in l[1:]])
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
    def __init__(name, thickness, sensitive = False) :
        self.name = name
        self.thickness = thickness
        self.sensitive = sensitive

    def air(t) :
        return Layer('Air',t)

    def tungsten(t) :
        return Layer('W', t)

    def pcb() :
        return Layer('PCB',1.666)

    def glue(t) :
        return Layer('Glue', t)

    def silicon() :
        return Layer('Si',0.5,sensitive=True)

    def carbon() :
        return Layer('Carbon',5.7)

    def enumerate_full_stack(sections) :
        layers = []
        for name, num_bilayers, front, cooling in sections :
            for bilayer in range(num_bilayers) :
                layers.append(Layer.air(0.5))
                if front > 0 :
                    layers.append(Layer.tungsten(front))
                    layers.append(Layer.air(0.5)
                layers.append(Layer.pcb())
                if cooling == 0 :
                    layers.append(Layer.air(0.5))
                layers.append(Layer.air(3.5))
                layers.append(Layer.glue(0.1))
                layers.append(Layer.silicon())
                layers.append(Layer.glue(0.2))
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.carbon())
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.glue(0.2))
                layers.append(Layer.silicon())
                layers.append(Layer.glue(0.1))
                layers.append(Layer.air(3.5))
                if cooling == 0 :
                    layers.append(Layer.air(0.5))
                layers.append(Layer.pcb())
    
        return layers

def main() :
    # Section, bilayers, front, cooling
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

    layers = Layer.enumerate_full_stack(('ps',1,0,0)+absorber_sections)

