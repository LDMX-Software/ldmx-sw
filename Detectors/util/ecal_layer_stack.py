"""Calculate the depth parameters for the ECal absorber layers"""

def print_gdml_list(**kwargs) :
    for name, l in kwargs.items() :
        newline_list = f'{l[0]:.1f}\n'
                     + '\n'.join([f'                {v:.1f}' for v in l[1:]])
        print(f'<matrix name="{name}"')
        print( '        coldim="1"')
        print(f'        values="{newline_list}"/>')


# Section, bilayers, front, cooling
sections = [
        ('a',1,1,1),
        ('b',1,2,1.5),
        ('c',9,3.5,1.8),
        ('d',5,7,3.5)
        ]

cooling_tungsten_dz = []
front_tungsten_dz = []
bilayer_absorber_cumulative = [0.]
for name, num_bilayers, front, cooling in sections :
    for bilayer in range(num_bilayers) :
        cooling_tungsten_dz.append(cooling)
        front_tungsten_dz.append(front)
        bilayer_absorber_cumulative.append(bilayer_absorber_cumulative[-1] + 2*cooling + front)

print_gdml_list(cooling_tungsten_dz = cooling_tungsten_dz,
                front_tungsten_dz = front_tungsten_dz,
                bilayer_absorber_cumulative = bilayer_absorber_cumulative)
