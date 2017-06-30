Geant4_DIR=/nfs/slac/g/ldmx/software/geant4/rhel6-64/geant4.10.02.p02-install/lib64/Geant4-10.2.2/
ROOT_DIR=/nfs/slac/g/ldmx/software/root/rhel6-64/root-6.06.08-install/
XercesC_DIR=/nfs/slac/g/ldmx/software/xerces/rhel6-64/xerces-c-3.1.4/
PYTHONHOME=/nfs/slac/g/ldmx/software/python/rhel6-64/python-2.7.13-install/

cmake -DGeant4_DIR=$Geant4_DIR -DROOT_DIR=$ROOT_DIR -DXercesC_DIR=$XercesC_DIR -DCMAKE_INSTALL_PREFIX=../install \
      -DPYTHON_EXECUTABLE=$PYTHONHOME/bin/python -DPYTHON_INCLUDE_DIR=$PYTHONHOME/include/python2.7 -DPYTHON_LIBRARY=$PYTHONHOME/lib/libpython2.7.so ..
