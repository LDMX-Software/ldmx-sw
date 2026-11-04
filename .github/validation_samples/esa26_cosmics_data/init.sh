#!/bin/bash

start_group decompress ESA26 calorimeter raw data
_calo_dir=${CI_DATA}/esa26_cosmics_data
ls ${_calo_dir}/*.dat.xz | xargs -P 4 -n 1 xz -d -k -f
ls -l ${_calo_dir}/*.dat
end_group
