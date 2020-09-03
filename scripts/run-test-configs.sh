#!/bin/bash

###############################################################################
# run-test-configs.sh
#   Run all the test configs and error out if any of them fail to run
#
#   This set written to be run automatically by the GitHub testing action.
#   So it assumes that the runner is using docker.
###############################################################################

cd ${LDMX_BASE}

failed_configs=""
for config in `ls ldmx-sw/*/test/*.py`
do
    if docker run --rm -it -e LDMX_BASE -v $(pwd):$(pwd) ldmx/dev $(pwd) fire "$config"
    then
        echo "${config} fired successfully."
    else
        echo "fire ${config} failed."
        failed_configs="${failed_configs},${config}"
    fi
done

if [[ -z "$failed_configs" ]]
then
    #failed_configs is empty ==> nothing failed
    return 0
else
    echo "Failed configs: ${failed_configs}"
    return 1
fi
