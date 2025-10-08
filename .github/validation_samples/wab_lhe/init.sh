#!/bin/bash

start_group Download and unzip compressed LHE files as input 
wget https://raw.githubusercontent.com/LDMX-Software/ci-data/refs/heads/main/wab_lhe/8GeV_WABFF2_10K.lhe.gz
gzip -d 8GeV_WABFF2_10K.lhe.gz
end_group
