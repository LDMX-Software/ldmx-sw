# Geometry name
geo_name = 'reduced_v3'

# Constants
PCB_dz = 1.666
Glue_dz = 0.1
Si_dz = 0.3 # Only 0.12 is active Si width
GlueThick_dz = 0.2
CarbonBasePlate_dz = 0.79
MotherBoardAssembly_dz = 8.166
CarbonCoolingPlane_dz = 3.18
strongback_bar_dy = 25.4
strongback_bar_dx = 838.2-0.6815
PCB_Motherboard_Gap = 3.5
FrontTolerance = 0.5
BackTolerance = 0.5
preshower_extra_air = 0.5
bilayer_start = 240
sampling_section_offset = 2*preshower_extra_air
front_pre_tungsten_dz = 2.0
cooling_pre_tungsten_dz = 2.0
correction_constant = 0.15
test_module_tolerance = 0 # Specific to reduced_v3 geo
sampling_section_tolerance = 0 # Specific to reduced_v3 geo
module_two_tolerance = 0 # Specific to reduced_v3 geo
module_four_tolerance = 0 # Specific to reduced_v3 geo

# Arrays for v15 geometry
layer_z_positions = [
    7.582, 16.062, 33.226, 43.206, 60.370, 71.350, 90.014, 101.594,
    120.258, 131.838, 150.502, 162.082, 180.746, 192.326, 210.990, 222.570,
    241.234, 252.814, 271.478, 283.058, 301.722, 313.302, 331.966, 343.546,
    365.710, 380.690, 402.854, 417.834, 439.998, 454.978, 477.142, 492.122,
    514.286, 529.266]
bilayer_absorber_cumulative = [
    0.0, 3.0, 8.0, 15.1, 22.2, 29.3, 36.4, 43.5, 50.6,
    57.7, 64.8, 71.9, 85.9, 99.9, 113.9, 127.9, 141.9]
front_tungsten_dz = [
    1.0, 2.0, 3.5, 3.5, 3.5, 3.5, 3.5, 3.5,
    3.5, 3.5, 3.5, 7.0, 7.0, 7.0, 7.0, 7.0]
cooling_tungsten_dz = [
    1.0, 1.5, 1.8, 1.8, 1.8, 1.8, 1.8, 1.8,
    1.8, 1.8, 1.8, 3.5, 3.5, 3.5, 3.5, 3.5]

# Arrays/constants for reduced_v3 geo (changing geo_name will cause script to go back to
# v15)
if geo_name == 'reduced_v3':
    layer_z_positions = [0, 0, 0, 0]
    bilayer_absorber_cumulative = [0.0] * 2
    front_tungsten_dz = [0.0] * 2
    cooling_tungsten_dz = [0.0] * 2
    CarbonBasePlate_dz = 0.0
    test_module_tolerance = 86-7.582
    sampling_section_tolerance = 136.8-108.394
    module_two_tolerance = 92.23-90.38
    module_four_tolerance = 143.03-140.68

# Calculated constants
Flower_dz = PCB_dz+Glue_dz+Si_dz+GlueThick_dz+CarbonBasePlate_dz
bilayer_noabsorber_thickness = (
    FrontTolerance + PCB_dz + PCB_Motherboard_Gap + Flower_dz
    + CarbonCoolingPlane_dz + Flower_dz + PCB_Motherboard_Gap
    + PCB_dz + BackTolerance)

# Calculate positions of silicon modules
first_sc_z_pos = (
    bilayer_start + test_module_tolerance + FrontTolerance
    + PCB_dz + PCB_Motherboard_Gap + PCB_dz + Glue_dz + correction_constant)
second_sc_z_pos = (
    bilayer_start + test_module_tolerance + FrontTolerance
    + PCB_dz + PCB_Motherboard_Gap + Flower_dz + preshower_extra_air
    + module_two_tolerance + CarbonCoolingPlane_dz + CarbonBasePlate_dz
    + GlueThick_dz + correction_constant)
calc_layers = [first_sc_z_pos, second_sc_z_pos]
for i in range(len(bilayer_absorber_cumulative)-1):
    odd_layer = (
        bilayer_start + test_module_tolerance + sampling_section_offset
        + bilayer_noabsorber_thickness*(i+1) + bilayer_absorber_cumulative[i]
        + BackTolerance + module_two_tolerance + sampling_section_tolerance
        + front_tungsten_dz[i] + FrontTolerance + PCB_dz
        + PCB_Motherboard_Gap + PCB_dz + Glue_dz + correction_constant)
    even_layer = (
        bilayer_start + test_module_tolerance + sampling_section_offset
        + bilayer_noabsorber_thickness*(i+1) + bilayer_absorber_cumulative[i]
        + BackTolerance + module_two_tolerance + sampling_section_tolerance
        + front_tungsten_dz[i] + FrontTolerance + PCB_dz + PCB_Motherboard_Gap
        + Flower_dz + module_four_tolerance + cooling_tungsten_dz[i]
        + CarbonCoolingPlane_dz + cooling_tungsten_dz[i]
        + CarbonBasePlate_dz + GlueThick_dz + correction_constant)
    calc_layers.append(odd_layer)
    calc_layers.append(even_layer)

# print results
print(f"For geometry: {geo_name}")
print("GDML | EcalGeometry.py | Diff")
for i in range(len(bilayer_absorber_cumulative)*2):
    print(round(calc_layers[i] - 240, 3), round(layer_z_positions[i], 3),
          round(abs(calc_layers[i] - layer_z_positions[i] - 240), 3))
