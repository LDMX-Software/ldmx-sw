###############################################################################
# @file track_filters.py
# These UserActions are used to filter tracks during the simulation
#   i.e. They choose tracks to be persisted (including the default ones)
#
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

from LDMX.SimApplication import simcfg

###############################################################################
# @func keepTracksMadeBy
# Returns a filter that keeps tracks made by the input process
###############################################################################
def keepTracksMadeBy( processName ) :
    track_process_filter = simcfg.UserAction( 'keep_' + processName + '_children' , "ldmx::TrackProcessFilter")
    track_process_filter.parameters['process'] = processName
    return track_process_filter

###############################################################################
# @func keepPNTracks
# Returns a filter that keeps tracks made by photonNuclear interactions
###############################################################################
def keepPNTracks( ) :
    return keepTracksMadeBy( 'photonNuclear' )

###############################################################################
# @func keepENTracks
# Returns a filter that keeps tracks made by electronNuclear interactions
###############################################################################
def keepENTracks( ) :
    return keepTracksMadeBy( 'electronNuclear' )

###############################################################################
# @func keepDarkTracks
# Returns a filter that keeps tracks made by dark brem interactions
###############################################################################
def keepDarkTracks( ) :
    return keepTracksMadeBy( 'eDBrem' )

