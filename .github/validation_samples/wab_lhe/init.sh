#!/bin/bash

start_group unzip compressed LHE files as input 
gzip -d ${CI_DATA}/wab_lhe/8GeV_WABFF2_10K.lhe.gz
end_group
