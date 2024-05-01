## Set the top level module
set_top TSTriggerAlgo

## Add source code
add_files [glob -directory ${PROJ_DIR}/src *]

## Add testbed files
add_files -tb [glob -directory ${PROJ_DIR}/tb *]
