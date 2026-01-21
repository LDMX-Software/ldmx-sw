import sys
import os

abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

'''

Author: CJ Barton / Lund University group
Added to ldmx-sw Jan 2026
Github PR 1888
Github issues 1760 & 1761

Script which automatically generates GDML files for the TS pads and their associated instruments.
Currently, 'associated instruments' are the SiPMs and the light pipes.

This script exists because of an incompatibility between the ldmx-sw design and Geant4/GDML.

In ldmx-sw, although the geometries are identical for the TS pads, the 3 TS pads in the baseline
are considered distinct entities (TSPad1, TSPad2, TSPad3). These entities have their own
hit collections, processors etc. and are hard-coded with distinct names in sensitive_detectors.py.
The geometry implementation is done in 3 loops across 2 files (trig_scint.gdml for TSPad1/2
and target.gdml for TSPad3). This means that any time a change needs to be made to the dimensions
or components of the TSPads, it must be changed 3 times.

This is generally bad practice, and GDML has a solution for copying the same geometry into
multiple locations, the <assembly>. A geometry defined in a sub-file can be implemented into
the current GDML file as a complete object and then placed multiple times. This is already done
for the ecal_motherboard(5/6)assembly.gdml files, and in principle this should be done for the
TSPads as well. 

A problem arises when we try to make the TSPads imported via <assembly> into distinct sensitive
detectors (SDs), when the assemblies are intentionally identical.
The parameter used to assign SDs is the G4LogicalVolume. In ldmx-sw, the correct G4LogicalVolume
is found by searching the logical volume store for the intended SD names. For the assemblies, we
can distinguish the physical volumes - but there is absolutely no way (as far as I know) to
rename the logical volumes. So when we assign a component in the assembly as an SD, this
assignment automatically spreads to the other assemblies and groups them all together.

This script is a compromise, to collect all of the parameters relevant to the TSPads in
one location while maintaining the distinct definitions for the logical volumes in each pad.
It still generates <assembly> files for import into the relevant scripts, but now each pad
has its own <assembly> file with its own logical volume names.

It should be noted that this script is only concerned with differentiating TSPads+instruments.
External factors, like where to place them in the world or the definitions of the materials,
should still be defined elsewhere, for example in the constants.gdml or materials.gdml files.
'''

#Material variable names (material definitions should be found in materials.gdml)
scintillator_mat="Polyvinyltoluene"
lightpipe_mat   ="AcrylicPMMA" 
sipm_mat        ="Silicon"

for i in range(3):

    #Variables that must change between iterations
    filename = f"tspad{i+1}_assembly.gdml"
    scintillator_lvname=f"trigger_pad{i+1}_bar_volume"
    lightpipe_lvname=f"tspad{i+1}_lightpipe_volume"
    sipm_lvname=f"tspad{i+1}_sipm_volume"
    with open(filename,"w") as f:
        f.write(f'''<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<!DOCTYPE gdml [
<!ENTITY constants SYSTEM "constants.gdml">
]>


<!--
The trigger scintillator pads comprise of, at this time (Oct 2025):
2X24 PVT scintillator bars
2X24 PMMA light pipes
2X24 SiPM arrays (1 SiPM per bar)

For now all vertical surfaces are flush with no space left in between
Horizontal surfaces have gaps (the spaces between the bars)

Todo: add the plastic casing, add the metal feet which attach the bars
to the housing, and maybe see if the visattributes can be reworked
-->


<gdml xmlns:gdml="http://cern.ch/2001/Schemas/GDML" 
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" 
    xsi:noNamespaceSchemaLocation="gdml.xsd" >

  
  <define>
    &constants;    
    <variable name="x" value="1"/>
  </define>

  
  <solids>
    <!--The box encompassing the assembly-->
    <box lunit="mm" name="one_pad_box" x="trigger_bar_dx*2+sipm_thickness" y="(trigger_bar_dy+trigger_pad_bar_gap)*number_of_bars*2" z="trigger_pad_thickness"/>
    <!--The scintillator bars/light pipes dimensions-->
    <box lunit="mm" name="trigger_bar_box" x="trigger_bar_dx" y="trigger_bar_dy" z="trigger_pad_bar_thickness" />
    <!--SiPM dimensions-->    
    <box lunit="mm" name="trigger_sipm_box" x="sipm_thickness" y="sipm_dy" z="sipm_dz" />
  </solids>

  
  <structure>

    <!--TS pad is made of polyvinyl toluene-->
    <volume name="{scintillator_lvname}">
      <materialref ref="{scintillator_mat}"/>
      <solidref ref="trigger_bar_box"/>
      <auxiliary auxtype="VisAttributes" auxvalue="TriggerPadVis"/>
      <auxiliary auxtype="DetElem" auxvalue="TriggerPad"/>
    </volume>

    <!--TS light guides are made of acrylic PMMA
	For now, they have the exact same dimensions as the TS pad bars-->
    <volume name="{lightpipe_lvname}">
      <materialref ref="{lightpipe_mat}"/>
      <solidref ref="trigger_bar_box"/>
      <auxiliary auxtype="VisAttributes" auxvalue="TriggerPadVis"/>
      <auxiliary auxtype="DetElem" auxvalue="TriggerPad"/>
    </volume>

    <!--I'll let you guess what the SiPMs are made of :^)
	The model is the Hamamatsu S13360-2050VE-->
    <volume name="{sipm_lvname}">
      <materialref ref="{sipm_mat}"/>
      <solidref ref="trigger_sipm_box"/>
      <auxiliary auxtype="VisAttributes" auxvalue="TriggerPadVis"/>
      <auxiliary auxtype="DetElem" auxvalue="TriggerPad"/>
    </volume>
    
    <volume name="tspad{i+1}_volume">
      <materialref ref="Vacuum"/>
      <solidref ref="one_pad_box" />

      
<!--The entire assembly is aligned with the left edge of the encompassing
    volume. This is to keep the encompassing volume as small as possible.
    Unfortunately, this makes positioning the assembly more awkward in
    the other files. The alternative was to make the envelope much larger
    in x, and have x=0 align with the center of the scintillating bars.
    This would likely cause overlaps with other pieces of the geometry
    eventually, namely the support structure holding the TS and target etc

    The bright side is that, for now, the assembly x=0 is almost aligned
    at the juncture between the scintillating bars and the light pipes.
-->
      
      <loop for="x" to="number_of_bars" step="1">

	<physvol copynumber="2*x-2">
          <volumeref ref="{scintillator_lvname}" />
          <position name="trigger_pad_bar_layer1_pos" unit="mm"
		    x="-(trigger_bar_dx+sipm_thickness)/2"
                    y="-target_dim_y/2 + trigger_bar_dy*(x - 0.5) + trigger_pad_bar_gap*(x - 1) + trigger_pad_offset" 
                    z="-trigger_pad_bar_thickness/2 - trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>
	
        <physvol copynumber="2*x - 1">
          <volumeref ref="{scintillator_lvname}" />
          <position name="trigger_pad_bar_layer2_pos" unit="mm"
		    x="-(trigger_bar_dx+sipm_thickness)/2"
                    y="-target_dim_y/2 + trigger_bar_dy*x + trigger_pad_bar_gap*(x - 1) + trigger_pad_offset"
                    z="trigger_pad_bar_thickness/2 + trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>

	<physvol copynumber="2*x-2">
	  <volumeref ref="{lightpipe_lvname}" />
          <position name="trigger_pad_bar_layer1_pos" unit="mm" 
          	    x="(trigger_bar_dx-sipm_thickness)/2"
                    y="-target_dim_y/2+trigger_bar_dy*(x-0.5)+trigger_pad_bar_gap*(x-1)+trigger_pad_offset" 
                    z="-trigger_pad_bar_thickness/2 - trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>
	
        <physvol copynumber="2*x-1">
          <volumeref ref="{lightpipe_lvname}" />
          <position name="trigger_pad_bar_layer2_pos" unit="mm" 
          	    x="(trigger_bar_dx-sipm_thickness)/2"
                    y="-target_dim_y/2+trigger_bar_dy*x+trigger_pad_bar_gap*(x-1)+trigger_pad_offset"
                    z="trigger_pad_bar_thickness/2 + trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>
	
	<physvol copynumber="2*x-2">
	  <volumeref ref="{sipm_lvname}" />
          <position name="trigger_sipm_layer1_pos" unit="mm" 
          	    x="trigger_bar_dx+sipm_thickness/2"
                    y="-target_dim_y/2+trigger_bar_dy*(x-0.5)+trigger_pad_bar_gap*(x-1)+trigger_pad_offset" 
                    z="-trigger_pad_bar_thickness/2 - trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>
	
        <physvol copynumber="2*x-1">
          <volumeref ref="{sipm_lvname}" />
          <position name="trigger_sipm_layer2_pos" unit="mm" 
          	    x="trigger_bar_dx+sipm_thickness/2"
                    y="-target_dim_y/2+trigger_bar_dy*x+trigger_pad_bar_gap*(x-1)+trigger_pad_offset"
                    z="trigger_pad_bar_thickness/2 + trigger_pad_bar_gap/2" />
          <rotationref ref="identity" />
        </physvol>
      </loop>

      <auxiliary auxtype="Region" auxvalue="trig_scint" />
      <!--<auxiliary auxtype="VisAttributes" auxvalue="TriggerPadVis"/>-->
      <auxiliary auxtype="DetElem" auxvalue="TriggerPad"/>

    </volume>
  </structure> 

  
  <setup name="Default" version="1.0">
    <world ref="tspad{i+1}_volume"/>
  </setup>

</gdml>
''')

