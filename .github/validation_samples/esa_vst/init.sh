#!/bin/bash

start_group assemble ESA VST raw data from compressed parts
_vst_dir=${CI_DATA}/esa_vst
ls ${_vst_dir}/run111_part*.dat.xz | xargs -P 4 -n 1 xz -d
# append and delete one part at a time to limit disk usage
> ${_vst_dir}/run111.dat
for _part in ${_vst_dir}/run111_part*.dat; do
  cat ${_part} >> ${_vst_dir}/run111.dat
  rm ${_part}
done
ls -l ${_vst_dir}/run111.dat
end_group
