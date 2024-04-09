"""Calculate the depth parameters for the ECal absorber layers"""

import sys

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

    dEdx = {
        'Al'     : (2.699 * 1.615) / 10,
        'Air'    : (1.815 * 1.205) / 10,
        'PCB'    : (1.815 * 1.205) / 10,
        'Si'     : (2.329 * 1.664) / 10,
        'W'      : (19.3  * 1.145) / 10,
        'Carbon' : (1.742 * 2.210) / 10,
        'Glue'   : (1.815 * 1.205) / 10
    }
    
    X0 = {
        'Al'     : ( 24.01 / 2.699 ) * 10,
        'Air'    : ( 36.62 / 1.205 ) * 10,
        'PCB'    : ( 36.62 / 1.205 ) * 10,
        'Si'     : ( 21.82 / 2.329 ) * 10,
        'W'      : ( 6.76  / 19.3  ) * 10,
        'Carbon' : ( 42.70 / 2.210 ) * 10,
        'Glue'   : ( 36.62 / 1.205 ) * 10
    }
    
    nuclen = {
        'Al'     : ( 107.2 / 2.699 ) * 10,
        'Air'    : ( 90.1  / 1.205 ) * 10,
        'PCB'    : ( 90.1  / 1.205 ) * 10,
        'Si'     : ( 108.4 / 2.329 ) * 10,
        'W'      : ( 191.9 / 19.3  ) * 10,
        'Carbon' : ( 85.8  / 2.210 ) * 10,
        'Glue'   : ( 90.1  / 1.205 ) * 10
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
                layers.append(Layer.air(0.5))
                if front > 0 :
                    layers.append(Layer.tungsten(front))
                    layers.append(Layer.air(0.5))
                layers.append(Layer.pcb()) # Motherboard
                if cooling == 0 :
                    layers.append(Layer.air(0.5))
                layers.append(Layer.air(3.5))
                layers.append(Layer.pcb())
                layers.append(Layer.glue(0.1))
                layers.append(Layer.silicon())
                layers.append(Layer.glue(0.2))
                layers.append(Layer.carbon(0.79))
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.carbon(5.7))
                if cooling > 0 :
                    layers.append(Layer.tungsten(cooling))
                layers.append(Layer.carbon(0.79))
                layers.append(Layer.glue(0.2))
                layers.append(Layer.silicon())
                layers.append(Layer.glue(0.1))
                layers.append(Layer.pcb())
                layers.append(Layer.air(3.5))
                if cooling == 0 :
                    layers.append(Layer.air(0.5))
                    layers.append(Layer.air(0.75))
                layers.append(Layer.pcb())
    
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

    layers = Layer.enumerate_full_stack([('ps',1,0,0)]+absorber_sections)
    mbs = materials_between_sensdet(layers)
    weights = calc_weights(mbs)
    print_weights(*weights)

    for l in layers :
        print(l)

if __name__ == '__main__' :
    main()
