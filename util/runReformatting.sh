#!/bin/bash


#a script to run reformatting on all files in a file list

#some defaults
nSamp=30
startSamp=0
nEvents=-1
doVerbose=0
startDoRerun=0
trigFlag="p"
deadChannel=-1 #8   #sometimes elecID 8 is not really connected
recoSamp=14
timeOffset=3   #offset in nTimeSamples for fiber2
RUN_FIRE='ldmx fire'
channelMap="channelMap_LYSOback_plasticFront_12-to-16channels_rotated180.txt" #<--- use this for 2022 data 
#"channelMap_LYSOback_plasticFront_12-to-16channels_rotated180_fiberSwap.txt" #<--- use this for mar28 2022 data
#"channelMap_LYSOback_plasticFront_12-to-16channels.txt"                      #<--- october 2021 test beam data 
#"channelMap_identity_16channels.txt"                                         #<--- this gives chanID = elecID 
fileList=""
OPTIND=1 #reset option counter between runs

while getopts 'f:hs:n:e:t:d:r:o:c:VRS' flag; do
  case "${flag}" in
    h) echo  "usage: (note boolean flags with capital letters) \n. $0 [-f file list (mandatory)] 
	\t[-s startSample (default: $startSamp)] 
	\t[-n numberTimeSamples (default: $nSamp)] 
	\t[-e number of events (default: $nEvents, for all)]  \n\t[-t p(def)/a/t, exclusive boolean trigger option flags: passThrough (default)/adcTrig/tdcTrig ] \n\t[-d dead channel nb (default: $deadChannel) \n\t[-r hit reco time sample (default: $recoSamp)]  \n\t[-o time offset for fiber 2 (default: $timeOffset)]  \n\t[-c channel map name (default: $channelMap)]  \n\t[-V verbose boolean ] \n\t[-R rerun from first step ] \n\t[-S run with singularity (sets appropriate fire command)] "
       #exit 0;;   # works with sh, but then can't run the container
       return 0;; # works with source or .
    f) fileList="${OPTARG}" ; echo $fileList;; 
    s) startSamp="${OPTARG}" ;;
    n) nSamp="${OPTARG}" ;;
    e) nEvents="${OPTARG}" ;;
    t) trigFlag="${OPTARG}" ;;
    d) deadChannel="${OPTARG}" ;;
    r) recoSamp="${OPTARG}" ;;
    o) timeOffset="${OPTARG}" ;;
    c) channelMap="${OPTARG}" ;;
    V) doVerbose=1 ;;
    R) startDoRerun=1 ;;
    S) RUN_FIRE='fire' ;;
    \?) echo "Unexpected option ${flag}" ; return 1 ;;
  esac
done

echo $fileList

if [ -z "${fileList}" ] ; then 
   echo "An input file list (listing the files to run over) must be specified. Use -f [file list]. Exiting."
   return 1;
fi



id="_reformat_${nSamp}timeSamplesFrom${startSamp}" #_fiberSwap"   #_oct2021mapping" #_fiberMapping"  #
logVerbosity=2 #set to 0 for lots of debug printout to logs, and rudimentary python decoding script plots 

#should convert this just to a string of flags to pass
if [[ "${trigFlag}" == "t" ]] ; then 
	id="${id}_tdcTrig"
elif [[ "${trigFlag}" == "a" ]] ; then
	id="${id}_adcTrig"
fi

echo "trigger flags set to ${trigFlag}"

echo "using identifier $id"

#we will assume a few env variables are set: LDMX_BASE (from sourcing ldmx-env.sh) and the appropriate fire command (prepended by ldmx only when running locally) set by the calling script. 
reformatDir=${PWD}
configDir=${LDMX_BASE}/ldmx-sw/TrigScint/exampleConfigs
utilDir=${LDMX_BASE}/ldmx-sw/TrigScint/util
digiDir=${LDMX_BASE}/ldmx-sw/run

ln -s ${configDir}/../data/${channelMap} ${channelMap}


for file in $(cat $fileList) ; do
	echo "Running over $file ..."
	sleep 1 
	timeStart=$(date +%s)                   #keep track of processing time  
	datFile="${file//.txt/$.dat}"           #agnostic to ascii or bin input at this point
	datFile="${file//.dat/${id}.dat}"       #reformat output name given by code 
	datLogFileName="${datFile//.dat/.log}"  #log file uses output name 
	rawRootFile="${datFile//.dat/.root}"    #output from RAW->tree format conversion 
	path=${file%/*}
	path="${path//raw/rootFiles}"           #set it to the final path for now. when running batch, instead use a tmp dir, and then copy
	fName=${rawRootFile##*/}
	#then follows all the stepwise ldmx producer output files
	digiRootFile="${path}/unpacked_${fName}"
	digiRootLogFile="${digiRootFile/.root/.log}"
	linearRootFile="${digiRootFile/.root/_linearize.root}" 
	hitsRootFile="${linearRootFile/.root/_hits.root}"
	clusterRootFile="${hitsRootFile/.root/_clusters.root}"
	doRerun=$startDoRerun
	if [[ ! -f $datFile || ${doRerun} -eq 1 ]] ; then
		echo "Producing reformatted file $datFile ..."
		python3 ${utilDir}/decode_2fibers_toRAW_fromBin.py -i $file -o $datFile -n ${nSamp} -N ${nEvents} -f ${startSamp} -${trigFlag} > $datLogFileName		
		doRerun=1
		#extract number of found events from log 
		nEvsRun=$(grep " events to .dat file" $datLogFileName | grep -Eo '[0-9]{1,6}')
		echo "Log reports $nEvsRun events written to .dat file"
	else
		echo "Already reformatted ${file} to ${datFile}; proceeding to the ldmx-sw steps."
	fi
	timeDat=$(date +%s)
	#doRerun=1     #sometimes convenient to force rerunning from here 
	if [[ ! -f ${rawRootFile} ||  ${doRerun} -eq 1 ]] ; then  #if no .root file with the right name, or, indeed did rerun the .dat step/should otherwise rerun from here
		echo "Running raw unpacking on ${datFile} to produce ${rawRootFile} ..."
		$RUN_FIRE $configDir/runRawUnpacker.py ${datFile} ${rawRootFile} ${nSamp} ${nEvents}
		doRerun=1
	else
		echo "Already unpacked ${datFile} to ${rawRootFile} ; proceeding to the decoding step."
		doRerun=0
    fi
	#doRerun=1     #sometimes convenient to force rerunning from here 
	timeRaw=$(date +%s)
	if [[ ! -f ${digiRootFile} || ${doRerun} -eq 1 ]] ; then  #if no .root file, or, indeed did rerun the raw step
		$RUN_FIRE $configDir/runQIEDecode.py ${rawRootFile} ${digiRootFile} "raw" ${nSamp} ${channelMap} ${logVerbosity} 
	else
		echo "Already converted ${rawRootFile} to ${digiRootFile} ; proceeding to the TS steps."
    fi
	#the rest of the steps are way faster (and more likely to have changed) so no need to check if they're redundant
	timeDecode=$(date +%s)
	timeReco=$(date +%s)
	$RUN_FIRE $configDir/runTSLinearizer.py ${digiRootFile} ${timeOffset} ${logVerbosity}
	timeLin=$(date +%s)
	gainFile="${linearRootFile/.root/_gains.txt}"
	if [[ "${trigFlag}" == "p" && ( ${nEvents} < 0 || ${nEvents} > 3000 ) && (! -f $gainFile || ${doRerun} -eq 1 ) ]] ; then   
	    #derive gains from: passthrough file, with large enough number of events for fitting, unless there is already one for this file 
	    #and we haven't asked to rerun everything from scratch 
		root -l -b -q ''$configDir'/../util/extractPedsAndGains.C+("'${linearRootFile}'", '${deadChannel}', "conv", '${logVerbosity}')'
	fi
	$RUN_FIRE $configDir/runTestBeamHitReco.py ${linearRootFile} ${recoSamp}
	timeHitReco=$(date +%s)
	$RUN_FIRE $configDir/runTestBeamClustering.py ${hitsRootFile}
	timeCluster=$(date +%s)
	$RUN_FIRE $configDir/runQIEana.py ${linearRootFile} ${startSamp}
	timeQIEana=$(date +%s)
	$RUN_FIRE $configDir/runTBHitana.py ${hitsRootFile} ${startSamp}
	timeEnd=$(date +%s)
	echo
	echo "**********************************************************"
	echo
	echo "Done reformatting $nEvsRun events from $file in $((timeEnd-timeStart)) seconds: "
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
		   
