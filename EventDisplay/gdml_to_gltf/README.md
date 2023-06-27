You can skip steps 1 and 2 if you already have a root file.
Converting from gdml to gltf:

    1) Remove any loops, array brackets, or references to other gdml files in the file you want to convert. This means flattening the loops, replacing the array bracket with the exact element from the array; So for the array "[1,2,3,4]", replace array[1] with 1, since gdml uses 1 based indexing, and replacing entity calls with the contents of the file.

    2) Convert gdml to root with gdml_to_root_export.cxx using the following command: "ldmx root -l gdml_to_root_export.cxx". Be sure the filename inside gdml_to_root_export.cxx is the correct gdml file

    3) Based on the following repo: https://github.com/HSF/root_cern-To_gltf-Exporter/tree/main, make an html file based off of export_LHCb.html, replacing filenames and arrays as needed. A working html file for the whole detector is included as export.html.

    4) The export.html file has 7 array for the parts it needs to generate. If you don't want to include a part, just comment out that line. This has been done for the magnet so it will only generate 6 parts. Uncomment that line if you want to see the magnet. Replace "true" with numbers ranging from 0 to 1 to change opacity.

    5) Put the new gltf file into phoenix: https://github.com/HSF/phoenix/tree/main

Example for this folder:
    ```
    ldmx root -l gdml_to_root_export.cxx
    
    python run
    ```
    Go to http://127.0.0.1:8000/export.html to download gltf file
    

