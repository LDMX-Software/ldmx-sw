import numpy as np

# define numbers and names we will use 
nLanes=14
nChan=6
nModules=4
mapFileName=f"../data/channelMap_{nModules}modules_{nLanes}lanes.txt"

# start calculating useful stuff
#nChanTotal=nLanes*nChan
# two lanes are for LYSO. discard those. knowing that there are
# three TS modules (pads), get the number of bars per TS module
nBarsPerModule=(nLanes-2)*nChan/3 


BAR_NUMBERS = np.array([
    [10, 0, 8, 6, 4, 2], # Lane 0
    [3, 11, 9, 5, 7, 1], 
    [22, 12, 20, 18, 16, 14],
    [15, 23, 21, 17, 19, 13],
    
    [10, 0, 8, 6, 4, 2], # Lane 4
    [3, 11, 9, 5, 7, 1], 
    [22, 12, 20, 18, 16, 14],
    [15, 23, 21, 17, 19, 13],
    
    [10, 0, 8, 6, 4, 2], # Lane 8
    [3, 11, 9, 5, 7, 1], 
    [22, 12, 20, 18, 16, 14],
    [15, 23, 21, 17, 19, 13],
    
    [9, 7, 11, 3, 1, 5], # Lane 12
    [15, 23, 21, 17, 19, 13]
])

# make an explicit mapping of lanes to physical modules.
# this code supports a chaotic cabling, as reality may dictate.
# incidentally, in ESA tests, cabled in order!
laneToModuleMap = [ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3]

#nElecIDs = nLanes*nChan

# open the file
with open(mapFileName, "w", encoding="utf-8") as f:
    # create the elecID from lane, module and chanNb
    barCounter=[0]*nModules
    for lane in range(nLanes) :
        module = laneToModuleMap[lane]
        print(f"At lane {lane}, module {module}")
        for iC in range(nChan) :
            #print(f"At channel {iC}, barCounter {barCounter}")
            elecID = 100*lane+10*module+iC
            barID = BAR_NUMBERS[lane][iC] 
            #print(f"-- elecID={elecID}, barID {barID}")
            f.write(f"{elecID}\t{barID}\n")
            # highlight when we reach the number of bars
            barCounter[module]+=1
            if barCounter[module] == nBarsPerModule :
                print(f"\t--- hitting end of module {module}")
    f.close()

moduleMapFileName=mapFileName.replace("channel", "module")
with open(moduleMapFileName, "w", encoding="utf-8") as f:
    for iL in range(nLanes) :
        f.write(f"{iL}\t{laneToModuleMap[iL]}\n")
    f.close()
