#!/usr/bin/env python
###################################################################
# Calculate layer weights and z position from list of layer materials and thicknesses
#   Input: List of materials in order from front to back of detector
#       Each material layer on new line, first entry is material name and second is material thickness [mm]
#           e.g: Si, 0.5  OR  Air, 4.0
#       The material names must match the names in the dictionaries below, so the material properties
#           can be looked up.
#   Ouput: Table containing estimates for dE/dx, X0, and Lambda for each sensitive layer
#       Layer numbers correspond to sensitive detector layers (in order from front to back)
#       Back corresponds to all material after the last sensitive detector layer
#
#   Review material properties to make sure they fit your situation.
#   Right now, the properties for Air are used for PCB and Glue because these layers are 
#   complicated but thin.

import argparse

parser = argparse.ArgumentParser("usage: %prog [options]")
parser.add_argument( "-i" , dest="inputFile" , help="File path to input list of materials. (default = 'materials.list')"
        + " The materials list should have each material layer on a new line with the form: name, thickness. "
        + "name must correspond to the dictionaries defined in the script and thickness must be in mm.",
        default = "materials.list" , type=str )
parser.add_argument( "-o" , dest="outputFile" , 
        help="File that will be over-written with the tabulated information (default = 'samplingfactors.list')." , 
        default = "samplingfactors.list" , type=str )
parser.add_argument( "-s" , dest="sensDetLayer" ,
        help="String that matches the sensitive detector layer in the materials list (required)." ,
        type=str )
arg = parser.parse_args()

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

# sensitive detector layer and thickness
SensDetLayer = arg.sensDetLayer
SensDetMaterial  = SensDetLayer.split(',')[0]
SensDetThickness = float(SensDetLayer.split(',')[1])

#### Helpful Functions ##################################################################
def getAveragedWeights(weights_raw):
    averagedWeights = [ ]
    for layerCounter in range(0,len(weights_raw)-1): # first to (n-1)th layers, where n is the total number of layers
        averagedWeights.append( 0.5*(weights_raw[layerCounter]+weights_raw[layerCounter+1]) )
    averagedWeights.append( weights_raw[-1] ) # last layer
    return averagedWeights

def getNormalized(weights):
    normalizationFactor = len(weights)/sum(weights)
    return [ weight*normalizationFactor for weight in weights ]

#### Get Sections of Material Layers from Input #########################################
materials_between_sensdet = [ ]
SensDetLayer = SensDetLayer + '\n'
with open( arg.inputFile , 'r') as mat_list:
    #split file into sections based off of sensitive detector material
    sections = mat_list.read().split( SensDetLayer )
    for section in sections:
        section = section.split('\n')[:-1]
        namesThicknesses = [ ] #names and thicknesses in this section
        for string in section:
            name = string.split(', ')[0]
            thick = float(string.split(', ')[1])
            if name :
                namesThicknesses.append( ( name , thick ) )
            #endif - name defined
        #endfor - strings in this section
        materials_between_sensdet.append(namesThicknesses)
    #endfor - section in list of sections
#file able to be opened

#### Perform Calculations ###############################################################
# Does not include sensitive detector layers
dE_between_sensdet = [ ]
X0_between_sensdet = [ ]
L_between_sensdet  = [ ]
# Does include sensitive detector layers
Zpos_layer = [ ]
for section in materials_between_sensdet:
    dE_section      = 0
    X0_section      = 0
    L_section       = 0
    Zdepth_section  = 0
    for name, thickness in section:
        dE_section      += thickness * dEdx[name]
        X0_section      += thickness / X0[name]
        L_section       += thickness / nuclen[name]
        Zdepth_section  += thickness
    #endfor - name,thickness pairs in this section

    dE_between_sensdet.append(dE_section)
    X0_between_sensdet.append(X0_section)
    L_between_sensdet.append(L_section)

    last_layer_pos = 0.0
    if len(Zpos_layer) > 0:
        last_layer_pos = Zpos_layer[-1] + SensDetThickness
    Zpos_layer.append( last_layer_pos + Zdepth_section )
#endfor - sections

dE_between_sensdet = getAveragedWeights(dE_between_sensdet)
X0_between_sensdet = getAveragedWeights(X0_between_sensdet)
L_between_sensdet  = getAveragedWeights(L_between_sensdet)

#### Print to Output File ###############################################################
with open( arg.outputFile , 'w' ) as output:
    output.write('{0:>5s} {1:>7s} {2:>6s} {3:>6s} {4:>6s}\n'.format('Layer', 'dE', 'X0', 'Lambda', 'Zpos'))
    for layer in range(len(materials_between_sensdet)-1):
        output.write('{0:5d} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
            layer+1, dE_between_sensdet[layer], X0_between_sensdet[layer], L_between_sensdet[layer], Zpos_layer[layer]))
    #endfor - layers
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Sum', sum(dE_between_sensdet[:-1]), sum(X0_between_sensdet[:-1]), sum(L_between_sensdet[:-1]), Zpos_layer[-1] ))
    output.write('{0:>5s} {1:7.3f} {2:6.3f} {3:6.3f} {4:6.3f}\n'.format(
        'Back', dE_between_sensdet[-1], X0_between_sensdet[-1], L_between_sensdet[-1], Zpos_layer[-1]))
#with output file opened

