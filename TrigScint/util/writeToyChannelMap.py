import numpy as np


# define numbers and names we will use
n_lanes = 14
n_chan = 6
n_modules = 4
map_file_name = f"../data/toyChannelMap_{n_modules}modules_{n_lanes}lanes.txt"

# start calculating useful stuff
n_chan_total = n_lanes * n_chan
# two lanes are for LYSO. discard those. knowing that there are
# three TS modules (pads), get the number of bars per TS module
n_bars_per_module = (n_lanes - 2) * n_chan / 3

# note that we are not treating LYSO differently here even though
# it has fewer instrumented bars -- not making sure we get
# 12 contiguous bar numbers. this is a toy map.
# if one wanted to, here's a place to start
# nChanInLYSO=2*n_chan

# make a sorted list of possible bar IDs
bar_ids = list(range(int(n_bars_per_module)))
print(f"Made bar_id list {bar_ids}")

map_list = [None] * n_modules
for i in range(n_modules):
    # this shuffles the original array but that's fine, we only
    # want four different versions
    new_order = np.random.permutation(bar_ids)
    map_list[i] = new_order
    print(f"module {i}: {map_list[i]}")

# make up some mapping of lanes to physical modules
lane_to_module_map = [0, 2, 1, 2, 0, 1, 0, 2, 2, 1, 1, 0, 3, 3]

n_elec_ids = n_lanes * n_chan

# open the file
with open(map_file_name, "w", encoding="utf-8") as f:
    # create the elec_id from lane, module and chanNb
    bar_counter = [0] * n_modules
    for i_l in range(n_lanes):
        lane = i_l
        module = lane_to_module_map[lane]
        print(f"At lane {lane}, module {module}")
        for i_c in range(n_chan):
            # print(f"At channel {i_c}, bar_counter {bar_counter}")
            elec_id = 100 * lane + 10 * module + i_c
            bar_id = map_list[module][bar_counter[module]]
            # print(f"-- elec_id={elec_id}, bar_id {bar_id}")
            f.write(f"{elec_id}\t{bar_id}\n")
            # highlight when we reach the number of bars
            bar_counter[module] += 1
            if bar_counter[module] == n_bars_per_module:
                print(f"\t--- hitting end of module {module}")
    f.close()

module_map_file_name = map_file_name.replace("Channel", "Module")
with open(module_map_file_name, "w", encoding="utf-8") as f:
    for i_l in range(n_lanes):
        f.write(f"{i_l}\t{lane_to_module_map[i_l]}\n")
    f.close()
