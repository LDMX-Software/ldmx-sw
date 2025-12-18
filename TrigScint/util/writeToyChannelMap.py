import numpy as np

# define numbers and names we will use 
nLanes=14
nChan=6
nModules=4
mapFileName=f"../data/toyChannelMap_{nModules}modules_{nLanes}lanes.txt"

# start calculating useful stuff
nChanTotal=nLanes*nChan
# two lanes are for LYSO. discard those. knowing that there are
# three TS modules (pads), get the number of bars per TS module
nBarsPerModule=(nLanes-2)*nChan/3 

# note that we are not treating LYSO differently here even though
# it has fewer instrumented bars -- not making sure we get
# 12 contiguous bar numbers. this is a toy map.
# if one wanted to, here's a place to start
# nChanInLYSO=2*nChan

#make a sorted list of possible bar IDs
barIDs= [i for i in range(int(nBarsPerModule))]
print(f'Made barID list {barIDs}')

mapList = [None]*nModules
for i in range(nModules) :
    # this shuffles the original array but that's fine, we only
    # want four different versions
    newOrder=np.random.permutation(barIDs)
    mapList[i] = newOrder
    print(f"module {i}: {mapList[i]}")

# make up some mapping of lanes to physical modules
laneToModuleMap = [ 0, 2, 1, 2, 0, 1, 0, 2, 2, 1, 1, 0, 3, 3]

nElecIDs = nLanes*nChan

# open the file
with open(mapFileName, "w", encoding="utf-8") as f:
    # create the elecID from lane, module and chanNb
    barCounter=[0]*nModules
    for iL in range(nLanes) :
        lane = iL
        module = laneToModuleMap[lane]
        print(f"At lane {lane}, module {module}")
        for iC in range(nChan) :
            #print(f"At channel {iC}, barCounter {barCounter}")
            elecID = 100*lane+10*module+iC
            barID = mapList[module][barCounter[module]]
            #print(f"-- elecID={elecID}, barID {barID}")
            f.write(f"{elecID}\t{barID}\n")
            # highlight when we reach the number of bars
            barCounter[module]+=1
            if barCounter[module] == nBarsPerModule :
                print(f"\t--- hitting end of module {module}")
    f.close()

moduleMapFileName=mapFileName.replace("Channel", "Module")
with open(moduleMapFileName, "w", encoding="utf-8") as f:
    for iL in range(nLanes) :
        f.write(f"{iL}\t{laneToModuleMap[iL]}\n")
    f.close()
