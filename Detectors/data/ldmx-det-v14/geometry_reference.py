import numpy as np

# Constants

hcal_envelope_dx = 2000.
hcal_envelope_dy = 2000.

air_thick = 2.
scint_thick = 20.
scint_width = 50.

hcal_back_dx = 2000.
hcal_back_dy = 2000.
hcal_back_numLayers = 96
hcal_back_numScint = 40
hcal_back_abso_thick = 25
hcal_back_layer_thick = hcal_back_abso_thick + scint_thick + 2.0*air_thick
hcal_back_dz = hcal_back_numLayers*hcal_back_layer_thick

hcal_sideTB_layers = 28
hcal_sideLR_layers = 26
hcal_side_abso_thick = 20.

hcal_side_dz = 600
hcal_dz = hcal_back_dz + hcal_side_dz

# first layer z position
back_start = hcal_side_dz/2.0 - hcal_back_dz/2.0

back_start_abso = back_start + air_thick + hcal_back_abso_thick/2.0
back_start_scint = back_start + air_thick + hcal_back_abso_thick + air_thick + scint_thick/2.0

back_startx_scint = -hcal_back_dx/2.0
back_starty_scint = -hcal_back_dy/2.0

hadron_calorimeter_parent_Box_pos = -hcal_back_dz/2.0

# first strip y position
back_start_scint_x = -hcal_back_dx/2.
back_start_scint_y = -hcal_back_dy/2.

# Solids
back_hcal_absoBox = {"width": hcal_back_dx,
                     "height": hcal_back_dy,
                     "depth": hcal_back_abso_thick}
back_hcal_AirBox = {"width": hcal_back_dx,
                    "height": hcal_back_dy,
                    "depth": air_thick}
back_hcal_ScintVerticalBox = {"width": hcal_back_dx,
                              "height": scint_width,
                              "depth": scint_thick}
back_hcal_ScintHorizontalBox = {"width": scint_width,
                                "height": hcal_back_dy,
                                "depth": scint_thick}
back_hadron_calorimeter_Box = {"width": hcal_envelope_dx,
                               "height": hcal_envelope_dy,
                               "depth": hcal_back_dz+hcal_side_dz}

class physical_volume:
    def __init__(self, position, name="", CopyNumber=None):
        self.position = position
        if CopyNumber is not None:
            self.CopyNumber = CopyNumber
            self.depth = position[2]

    def __repr__(self):
        return str(self.__dict__)

# Physical Volumes
absorber_physvols = []
horizontal_scint_physvols = []
vertical_scint_physvols = []

distance_to_subsequent_absorber_layer = hcal_back_layer_thick
distance_to_subsequent_scint_layer = 2*hcal_back_layer_thick

for i in range(1, hcal_back_numLayers + 1, 2):
    # absorbers
    absorber_physvols.append(physical_volume(position=[0,0,back_start_abso + distance_to_subsequent_absorber_layer * (i-1)],
				             name="back_hcal_abso_physvol",
                                             CopyNumber=i))

    # scintillators (horizontal parity = 1 - layers with odd parity (1) have horizontal strips)
    for j in range(1, hcal_back_numScint + 1, 1):
        horizontal_scint_physvols.append(physical_volume(position=[ 0,
                                                                    back_starty_scint + (j-1)*scint_width,
                                                                    back_start_scint + (i-1)*hcal_back_layer_thick],
                                                         name="back_hcal_scint_physvol",
                                                         CopyNumber=100*hcal_back_numLayers*hcal_back_numScint + 100*i + j))

    absorber_physvols.append(physical_volume(position=[0,0,back_start_abso + distance_to_subsequent_absorber_layer * (i)],
				             name="absorber_physvol",
                                             CopyNumber=i+1))

    for j in range(1, hcal_back_numScint + 1, 1):
        vertical_scint_physvols.append(physical_volume(position=[ back_startx_scint + (j-1)*scint_width,
                                                                  0,
                                                                  back_start_scint + (i-1)*hcal_back_layer_thick],
                                                       name="back_hcal_scint_physvol",
                                                       CopyNumber=200*hcal_back_numLayers*hcal_back_numScint + 100*i + j))
    
    
def absorber_copynumbers():
    return [absorber_physvols[i].CopyNumber
            for i in range(0, hcal_back_numLayers)]

def vertical_scint_copynumbers():
    #print('Vertical ',len(vertical_scint_physvols))
    copy_num = []
    for il,i in enumerate(range(2, hcal_back_numLayers+1, 2)):
        copy_num_layer = []
        for j in range(0, hcal_back_numScint):
            #print(hcal_back_numScint*il+j)
            copy_num_layer.append(vertical_scint_physvols[hcal_back_numScint*il+j].CopyNumber)
        copy_num.append(copy_num_layer)
    return copy_num

def horizontal_scint_copynumbers():
    #print('Horizontal ',len(horizontal_scint_physvols))
    copy_num = []
    for il,i in enumerate(range(1, hcal_back_numLayers+1, 2)):
        copy_num_layer = []
        for j in range(0, hcal_back_numScint):
            #print(hcal_back_numScint*il+j)
            copy_num_layer.append(horizontal_scint_physvols[hcal_back_numScint*il+j].CopyNumber)
        copy_num.append(copy_num_layer)
    return copy_num

print('Copy numbers')
print('Absorber: ',absorber_copynumbers())
print('Horizontal scintillator: ',horizontal_scint_copynumbers())
print('Vertical scintillator: ',vertical_scint_copynumbers())

# check that values are not repeated
horizontal = np.array(horizontal_scint_copynumbers())
vertical = np.array(vertical_scint_copynumbers())
all_scint = np.concatenate((horizontal, vertical), axis=None)

_, horizontal_counts = np.unique(horizontal, return_counts=True)
assert( np.all(horizontal_counts == 1))
_, vertical_counts = np.unique(vertical, return_counts=True)
assert( np.all(vertical_counts == 1))
_, all_counts = np.unique(all_scint, return_counts=True)
assert( np.all(all_counts == 1))
