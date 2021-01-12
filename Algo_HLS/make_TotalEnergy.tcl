# open the project, don't forget to reset
open_project -reset proj0
set_top TotalEnergy_hw
add_files src/TotalEnergy.cpp
add_files -tb tb/TotalEnergy_test.cpp 
add_files -tb ref/TotalEnergy_ref.cpp
add_files -tb data/test.dump

# reset the solution
open_solution -reset "solution1"
# part options:
#xcku9p-ffve900-2-i-EVAL
#xc7vx690tffg1927-2
#xcku5p-sfvb784-3-e
#xcku115-flvf1924-2-i
#xcvu9p-flga2104-2l-e
set_part {xcvu9p-flga2104-2l-e}
create_clock -period 5 -name default

# do stuff
csim_design
csynth_design
#cosim_design -trace_level all
#export_design -format ip_catalog  -vendor "cern-cms"

# exit Vivado HLS
exit
