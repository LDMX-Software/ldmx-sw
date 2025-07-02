#!/bin/bash

start_group Download files needed for GENIE
wget https://raw.githubusercontent.com/LDMX-Software/ci-data/refs/heads/main/target_genie/gxspl_emode_GENIE_v3_04_00.xml
wget https://github.com/LDMX-Software/ci-data/raw/refs/heads/main/target_genie/propagationMap.root
end_group
