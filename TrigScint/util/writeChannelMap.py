import numpy as np


# define numbers and names we will use
n_lanes = 14
n_chan = 6
n_modules = 4
map_file_name = f"../data/channelMap_{n_modules}modules_{n_lanes}lanes.txt"

# start calculating useful stuff
# n_chan_total=n_lanes*n_chan
# two lanes are for LYSO. discard those. knowing that there are
# three TS modules (pads), get the number of bars per TS module
n_bars_per_module = (n_lanes - 2) * n_chan / 3


BAR_NUMBERS = np.array(
    [
        [10, 0, 8, 6, 4, 2],  # Lane 0
        [3, 11, 9, 5, 7, 1],
        [22, 12, 20, 18, 16, 14],
        [15, 23, 21, 17, 19, 13],
        [10, 0, 8, 6, 4, 2],  # Lane 4
        [3, 11, 9, 5, 7, 1],
        [22, 12, 20, 18, 16, 14],
        [15, 23, 21, 17, 19, 13],
        [10, 0, 8, 6, 4, 2],  # Lane 8
        [3, 11, 9, 5, 7, 1],
        [22, 12, 20, 18, 16, 14],
        [15, 23, 21, 17, 19, 13],
        [9, 7, 11, 3, 1, 5],  # Lane 12
        [15, 23, 21, 17, 19, 13],
    ]
)

# make an explicit mapping of lanes to physical modules.
# this code supports a chaotic cabling, as reality may dictate.
# incidentally, in ESA tests, cabled in order!
lane_to_module_map = [0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3]

# n_elec_ids = n_lanes*n_chan

# open the file
with open(map_file_name, "w", encoding="utf-8") as f:
    # create the elec_id from lane, module and chanNb
    bar_counter = [0] * n_modules
    for lane in range(n_lanes):
        module = lane_to_module_map[lane]
        print(f"At lane {lane}, module {module}")
        for i_c in range(n_chan):
            # print(f"At channel {i_c}, bar_counter {bar_counter}")
            elec_id = 100 * lane + 10 * module + i_c
            bar_id = BAR_NUMBERS[lane][i_c]
            # print(f"-- elec_id={elec_id}, bar_id {bar_id}")
            f.write(f"{elec_id}\t{bar_id}\n")
            # highlight when we reach the number of bars
            bar_counter[module] += 1
            if bar_counter[module] == n_bars_per_module:
                print(f"\t--- hitting end of module {module}")
    f.close()

module_map_file_name = map_file_name.replace("channel", "module")
with open(module_map_file_name, "w", encoding="utf-8") as f:
    for i_l in range(n_lanes):
        f.write(f"{i_l}\t{lane_to_module_map[i_l]}\n")
    f.close()
