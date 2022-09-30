import numpy as np

# Constants

hcal_envelope_dx = 3000.
hcal_envelope_dy = 3000.

hcal_airThick = 2.
hcal_scintThick = 20.
hcal_scintWidth = 50.

back_hcal_numLayers = 96
back_hcal_numScint = 40
back_hcal_absoThick = 25
back_hcal_layerThick = back_hcal_absoThick + hcal_scintThick + 2.0*hcal_airThick
back_hcal_dx = 2000.
back_hcal_dy = 2000
back_hcal_dz = back_hcal_numLayers*back_hcal_layerThick

ecal_side_dx = 880.6815
ecal_side_dy = 600.

side_hcal_dz = 600.

side_hcal_lengthA = 1800.
side_hcal_lengthB = 1600.
side_hcal_lengthC = 1400.
side_hcal_lengthD = 1200.

side_hcal_numLayersA = 4
side_hcal_numLayersB = 3
side_hcal_numLayersC = 2
side_hcal_numLayersD = 3

side_hcal_numScintA = side_hcal_lengthA/hcal_scintWidth
side_hcal_numScintB = side_hcal_lengthB/hcal_scintWidth
side_hcal_numScintC = side_hcal_lengthC/hcal_scintWidth
side_hcal_numScintD = side_hcal_lengthD/hcal_scintWidth
side_hcal_numScintZ = side_hcal_dz/hcal_scintWidth

side_hcal_absoThick = 20.
side_hcal_numLayers = (side_hcal_numLayersA+side_hcal_numLayersB+side_hcal_numLayersC+side_hcal_numLayersD)*2
side_hcal_layerThick = side_hcal_absoThick + 2.*hcal_airThick + hcal_scintThick
side_hcal_moduleWidth = side_hcal_numLayers*side_hcal_layerThick
side_hcal_moduleLength = side_hcal_lengthA

hcal_envelope_dx = 3000.
hcal_envelope_dy = 3000.
hcal_envelope_dz = back_hcal_dz + side_hcal_dz
hcal_dz = back_hcal_dz + side_hcal_dz

back_hcal_startZ = side_hcal_dz/2.0 - back_hcal_dz/2.0
back_hcal_startZAbso = back_hcal_startZ + hcal_airThick + back_hcal_absoThick/2.0
back_hcal_startZScint = back_hcal_startZ + hcal_airThick + back_hcal_absoThick + hcal_airThick + hcal_scintThick/2.0
back_hcal_startXScint = -hcal_envelope_dx/2.0
back_hcal_startYScint = -hcal_envelope_dy/2.0

side_hcal_TB_startX = side_hcal_moduleWidth + ecal_side_dx/2.0
side_hcal_LR_startY = side_hcal_moduleWidth + ecal_side_dy/2.0

side_hcal_TB_startYAbso = ecal_side_dy/2.0 + hcal_airThick + side_hcal_absoThick/2.0
side_hcal_TB_startYScint = ecal_side_dy/2.0 + 2.0*hcal_airThick + side_hcal_absoThick + hcal_scintThick/2.0

hadron_calorimeter_parent_Box_pos = -back_hcal_dz/2.0

version = 1

# Solids
back_hcal_absoBox = {
    "width": back_hcal_dx,
    "height": back_hcal_dy,
    "depth": back_hcal_absoThick
}
back_hcal_scintXBox = {
    "width": back_hcal_dx,
    "height": hcal_scintWidth,
    "depth": hcal_scintThick,
}
back_hcal_scintYBox = {
    "width": hcal_scintWidth,
    "height": back_hcal_dy,
    "depth": hcal_scintThick,
}
back_hcal_Box = {
    "width": hcal_envelope_dx,
    "height": hcal_envelope_dy,
    "depth": hcal_dz,
}

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

for i in range(1, back_hcal_numLayers, 2):
    # absorbers
    absorber_physvols.append(
        physical_volume(
            position=[0,0,back_hcal_startZAbso + (i-1)*back_hcal_layerThick],
	    name="back_hcal_absoPhysvol",
            CopyNumber=i)
    )
    
    # scintillators (horizontal parity = 1 - layers with odd parity (1) have horizontal strips)
    for j in range(0, back_hcal_numScint, 1):
        horizontal_scint_physvols.append(
            physical_volume(
                position=[ 0,
                           back_hcal_startYScint + (j)*hcal_scintWidth,
                           back_hcal_startZScint + (i-1)*back_hcal_layerThick],
                name="back_hcal_scintXPhysvol",
                CopyNumber=version*256*256*256 + 0*256*256 + (i)*256 + j
            )
        )
        
    absorber_physvols.append(
        physical_volume(
            position=[0,0,back_hcal_startZAbso + (i)*back_hcal_layerThick],
	    name="back_hcal_absoPhysvol",
            CopyNumber=i+1)
    )

    for j in range(0, back_hcal_numScint, 1):
        vertical_scint_physvols.append(
            physical_volume(position=[ back_hcal_startXScint + (j)*hcal_scintWidth,
                                       0,
                                       back_hcal_startZScint + (i)*back_hcal_layerThick],
                            name="back_hcal_scintYPhysvol",
                            CopyNumber=version*256*256*256 + 0*256*256 + (i+1)*256 + j
                            )
        )

def absorber_copynumbers():
    return [absorber_physvols[i].CopyNumber
            for i in range(0, back_hcal_numLayers)]

def vertical_scint_copynumbers():
    print(f'Number of Vertical Volumes {len(vertical_scint_physvols)}')
    copy_num = []
    # loop over even layers
    for il,i in enumerate(range(2, back_hcal_numLayers+1, 2)):
        copy_num_layer = []
        for j in range(0, back_hcal_numScint):
            #print(back_hcal_numScint*il+j)
            copy_num_layer.append(vertical_scint_physvols[back_hcal_numScint*il+j].CopyNumber)
        copy_num.append(copy_num_layer)
    return copy_num

def horizontal_scint_copynumbers():
    print(f'Number of Horizontal Volumes {len(horizontal_scint_physvols)}')
    copy_num = []
    # loop over odd layers
    for il,i in enumerate(range(1, back_hcal_numLayers+1, 2)):
        copy_num_layer = []
        for j in range(0, back_hcal_numScint):
            # print(f'Index for scint {j}, layer {i} is {back_hcal_numScint*il+j}')
            copy_num_layer.append(horizontal_scint_physvols[back_hcal_numScint*il+j].CopyNumber)
        copy_num.append(copy_num_layer)
    return copy_num

def ref_back_hcal():
    absorber_copy_numbers = absorber_copynumbers()
    horizontal_copy_numbers = horizontal_scint_copynumbers()
    vertical_copy_numbers = vertical_scint_copynumbers()
    
    print('Copy numbers')  
    print('Absorber: ',absorber_copy_numbers)
    print('Horizontal scintillator: ',horizontal_copy_numbers)
    print('Vertical scintillator: ',vertical_copy_numbers)

    # check that values are not repeated
    horizontal = np.array(horizontal_copy_numbers)
    vertical = np.array(vertical_copy_numbers)
    all_scint = np.concatenate((horizontal, vertical), axis=None)
    
    _, horizontal_counts = np.unique(horizontal, return_counts=True)
    assert( np.all(horizontal_counts == 1))
    _, vertical_counts = np.unique(vertical, return_counts=True)
    assert( np.all(vertical_counts == 1))
    _, all_counts = np.unique(all_scint, return_counts=True)
    assert( np.all(all_counts == 1))

    
def ref_side_hcal():
    absorber_copy_numbers = absorber_copynumbers()


ref_side_hcal()
