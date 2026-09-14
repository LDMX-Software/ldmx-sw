#!/bin/bash

# a script to run reformatting on all files in a file list
# this is the ESA 2025 slice test data version

set -o errexit
set -o nounset

#some defaults
nSamp=30
startSamp=0
nEvents=-1
doVerbose=0
startDoRerun=0
deadChannel=-1 #8   #sometimes elecID 8 is not really connected
recoSamp=10
RUN_FIRE='just fire'
fileList=""
channelMap="channelMap_4modules_14lanes.txt" #<--- use this for 2025 data 
OPTIND=1 #reset option counter between runs

while getopts 'f:hs:n:e:d:r:c:o:VRS' flag; do
  case "${flag}" in
    h)
      cat <<HELP
usage: (note boolean flags with capital letters)
  $0 -f {file list (mandatory)}
    [-s startSample (default: $startSamp)] 
    [-n numberTimeSamples (default: $nSamp)] 
    [-e number of events (default: $nEvents, for all)]
    [-d dead channel nb (default: $deadChannel)]
    [-r hit reco time sample (default: $recoSamp)]
    [-c channel map name (default: $channelMap)]
    [-o output directory for root files (default: $rootFilePath)]
    [-V verbose boolean ]
    [-R rerun from first step ]
    [-S run with singularity (sets appropriate fire command)]
HELP
       exit 0
       ;;
    f) fileList="${OPTARG}" ;;
    s) startSamp="${OPTARG}" ;;
    n) nSamp="${OPTARG}" ;;
    e) nEvents="${OPTARG}" ;;
    d) deadChannel="${OPTARG}" ;;
    r) recoSamp="${OPTARG}" ;;
    c) channelMap="${OPTARG}" ;;
    o) outputDir="${OPTARG}" ;;
    V) doVerbose=1 ;;
    R) startDoRerun=1 ;;
    S) RUN_FIRE='fire' ;;
    \?)
      echo "Unexpected option ${flag}";
      exit 1
      ;;
  esac
done

if [ -z "${fileList}" ] ; then 
   echo "An input file list (listing the files to run over) must be specified. Use -f [file list]. Exiting."
   return 1;
fi

timeOffset=0
logVerbosity=2 #set to 0 for lots of debug printout to logs, and rudimentary python decoding script plots 
rootFilePath="rootFiles"

reformatDir=${PWD}
# deduce the location of ldmx-sw from the path to _this_ script
# requires to be run by bash
ldmx_sw_dir="$( cd -- "$(dirname -- "${BASH_SOURCE[0]}")/../../" && pwd)"
configDir=${ldmx_sw_dir}/TrigScint/exampleConfigs
utilDir=${ldmx_sw_dir}/TrigScint/util
digiDir=${ldmx_sw_dir}/run

# update map files to be full paths
channelMap="${configDir}/../data/${channelMap}"

for file in $(cat $fileList) ; do
  echo "Running over $file ..."
  sleep 1 
  timeStart=$(date +%s)                   #keep track of processing time  
  if [[ -n "${outputDir}" ]] ; then
    path="${outputDir}/${rootFilePath}"
  else
    path="${file%/*}/${rootFilePath}"
  fi
  mkdir -p $path
  echo "Using root file output path $path"
  fName=${file##*/}
  rawRootFile="${path}/${fName//.dat/.root}"    #output from RAW->tree format conversion 
  rootFileName=${rawRootFile##*/}
  #then follows all the stepwise ldmx producer output files
  digiRootFile="${path}/decoded_${rootFileName}"
  digiRootLogFile="${digiRootFile/.root/.log}"
  linearRootFile="${digiRootFile/.root/_linearize.root}" 
  hitsRootFile="${linearRootFile/.root/_hits.root}"
  clusterRootFile="${hitsRootFile/.root/_clusters.root}"
  doRerun=$startDoRerun
  timeDat=$(date +%s)
  #doRerun=1     #sometimes convenient to force rerunning from here 
  if [[ ! -f ${rawRootFile} ||  ${doRerun} -eq 1 ]] ; then  #if no .root file with the right name, or, indeed did rerun the .dat step/should otherwise rerun from here
    echo "Running raw unpacking on ${file} to produce ${rawRootFile} ..."
    $RUN_FIRE $configDir/runRawUnpackerZCCM.py ${file} ${rawRootFile} ${nEvents}
    doRerun=1
  else
    echo "Already unpacked ${file} to ${rawRootFile} ; proceeding to the decoding step."
    doRerun=0
    fi
  #doRerun=1     #sometimes convenient to force rerunning from here 
  timeRaw=$(date +%s)
  if [[ ! -f ${digiRootFile} || ${doRerun} -eq 1 ]] ; then  #if no .root file, or, indeed did rerun the raw step
    $RUN_FIRE $configDir/runZCCMDecode.py ${rawRootFile} ${digiRootFile} "raw" ${nSamp} ${channelMap} ${logVerbosity} 
  else
    echo "Already converted ${rawRootFile} to ${digiRootFile} ; proceeding to the TS steps."
    fi
  #the rest of the steps are way faster (and more likely to have changed) so no need to check if they're redundant
  timeDecode=$(date +%s)
  timeReco=$(date +%s)
  $RUN_FIRE $configDir/runTSEventProducer.py ${digiRootFile} ${timeOffset} ${logVerbosity}
  timeLin=$(date +%s)
  gainFile="${linearRootFile/.root/_gains.txt}"
  if [[ ( ${nEvents} < 0 || ${nEvents} > 3000 ) && (! -f $gainFile || ${doRerun} -eq 1 ) ]] ; then   
      #derive gains from: passthrough file, with large enough number of events for fitting, unless there is already one for this file 
      #and we haven't asked to rerun everything from scratch
      # parameters: (input file name, dead channel, pass name, number of samples, log verbosity, is_simulation, use only "clean" events, input collection)
      # TODO derive a gain file for every module and read that properly in hit reco
      root -l -b -q ''$configDir'/../util/extractPedsAndGains.C+("'${linearRootFile}'", '${deadChannel}', "conv", '${nSamp}', '${logVerbosity}', false, false, "QIEsamplesPad1")'
      
  fi
  $RUN_FIRE $configDir/runTestBeamHitReco.py ${linearRootFile} ${recoSamp}
  timeHitReco=$(date +%s)
  $RUN_FIRE $configDir/runTestBeamClustering.py ${hitsRootFile}
  timeCluster=$(date +%s)
  $RUN_FIRE $configDir/runZCCMana.py ${linearRootFile} ${startSamp}
  timeQIEana=$(date +%s)
  # TODO make work with these hits 
  #$RUN_FIRE $configDir/runTBHitana.py ${hitsRootFile} ${startSamp}
  timeEnd=$(date +%s)
  echo
  echo "**********************************************************"
  echo
  echo "Done reformatting $nEvents events from $file in $((timeEnd-timeStart)) seconds: "
  echo "txt -> dat in $((timeDat-timeStart)) s, "
  echo "dat -> raw root in $((timeRaw-timeDat)) s, "
  echo "raw root -> QIE in $((timeDecode-timeRaw)) s, "
  echo "QIE -> linear charge in $((timeLin-timeReco)) s."
  echo "linear charge -> testbeam hits in $((timeHitReco-timeLin)) s."
  echo "testbeam hits -> clusters in $((timeCluster-timeHitReco)) s."
  echo "event analysis in $((timeQIEana-timeCluster)) s."
  echo "hit analysis in $((timeEnd-timeQIEana)) s."
  echo
  echo "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-"
  echo 

  #now could copy selected output to some storage
  #leaving plot making to a separate script 
done
