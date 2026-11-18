"""Generates random run seeds and saves to seeds.json."""

import json
import numpy as np
import sys

MASTER_SEED = 12345
MAX_SEED = 2**31 - 1

rng = np.random.default_rng(MASTER_SEED)
num_seeds = int(sys.argv[1]) if len(sys.argv) > 1 else 10
seeds = rng.choice(MAX_SEED, size=num_seeds, replace=False).tolist()

with open("scripts/performance/seeds.json", "w") as f:
    json.dump({"master_seed": MASTER_SEED, "seeds": seeds}, f, indent=2)

print(f"Saved {len(seeds)} seeds to scripts/performance/seeds.json:")
print(seeds)
