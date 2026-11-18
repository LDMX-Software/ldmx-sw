""" Print and save performance data from hist.root file using uproot"""

import uproot
import sys
import json
import csv
import subprocess
import socket
import os
import numpy as np

USAGE_PATH = ".../run_<run>/hist_<mode>_<max_events>.root"

if (len(sys.argv) != 2): 
    raise SystemExit(f"Usage: {sys.argv[0]} {USAGE_PATH}")

# Get mode, run number, and max_events from hist file to name data files
hist_path = os.path.abspath(sys.argv[1])
if not os.path.isfile(hist_path):
    raise SystemExit(f"File not found: {hist_path}. Usage: {sys.argv[0]} {USAGE_PATH}")

try:
    hist_name = os.path.basename(hist_path)
    run_dir = os.path.basename(os.path.dirname(hist_path)) 
    qualifiers = hist_name.rsplit('.', 1)[0].split('_')
    MODE = qualifiers[1]
    MAX_EVENTS = int(qualifiers[2])
    RUN = int(run_dir.split('_')[1])
except(IndexError, ValueError):
    raise SystemExit(f"Unable to access mode, max_events, and run number from path. Usage: {sys.argv[0]} {USAGE_PATH}")

# just perf recipe runs git rev-parse HEAD and passes it in to program
SHA = os.environ.get("GIT_COMMIT_SHA")

def get_metadata(bool_completed):
    """ Collect and return metadata for the run, missing values are None"""

    # Find CPU
    cpu = None
    try:
        with open("/proc/cpuinfo") as f:
            for line in f:
                if line.startswith("model name"):
                    cpu = line.split(":", 1)[1].strip()
                    break
    except OSError:
        pass

    return {
        "git_commit_SHA": SHA,
        "container_image_tag": os.environ.get("DENV_IMAGE"),
        "container_runner": os.environ.get("DENV_RUNNER"),
        "benchmark_mode": MODE,
        "run": RUN,
        "completed_event_count": int(bool_completed.sum()),
        "max_event_count": MAX_EVENTS,
        "hostname": socket.gethostname(),
        "CPU_model": cpu,
    }

def write_summary(metadata, callback_rows, processor_rows, by_event_rows):
    """ Write by_event and summary json and csv files from hist.root file"""
    outdir = f"scripts/performance/data/run_{RUN}"
    os.makedirs(outdir, exist_ok=True)
    summary = {"metadata": metadata, "callbacks": callback_rows, "processors": processor_rows}
    
    with open (f"{outdir}/summary_{MODE}_{MAX_EVENTS}.json", "w") as o:
        json.dump(summary, o, indent=2)

    with open (f"{outdir}/summary_{MODE}_{MAX_EVENTS}.csv", "w", newline="") as o:
        writer = csv.DictWriter(o, fieldnames= list(processor_rows[0].keys()) + list(metadata.keys()))
        writer.writeheader() 
        for row in processor_rows:
            # Write metadata to every row
            row_w_metadata = {**row, **metadata}
            writer.writerow(row_w_metadata)

    with open (f"{outdir}/by_event_{MODE}_{MAX_EVENTS}.csv", "w", newline="") as o:
        writer = csv.DictWriter(o, fieldnames= list(by_event_rows[0].keys()) + list(metadata.keys()))
        writer.writeheader()
        for row in by_event_rows:
            row_w_metadata = {**row, **metadata}
            writer.writerow(row_w_metadata)


# Print Durations per Callback
print("Durations per Callback \n")
print("-" * 71)
print(f"| {'Callback':<15} | {'Processor':<32} | {'Duration (s)':<14} |")
print("-" * 71)

callback_rows = []
processor_rows = []
by_event_rows = []

with uproot.open(hist_path) as f:
    absolute = f['performance/absolute'].members

    by_event = f['performance/by_event']
    events = by_event.arrays(expressions=by_event.keys(recursive=False), library='np')

    bool_completed = events['completed']
        
    if not bool_completed.any():
        raise SystemExit("No completed events in hist.root file")
        
    print(f"| {'absolute':<15} | {'N/A':^32} | {absolute['duration_']:<14.3e} |")
    callback_rows.append({"callback": "absolute", "processor": "N/A", "duration_": float(absolute['duration_'])})
    for callback in [
        'onProcessStart', 'onProcessEnd',
        'onFileOpen','onFileClose',
        'beforeNewRun', 'onNewRun'
    ]:

        for name, timer in f[f'performance/{callback}'].items(cycle = False):
            print(f"| {callback:<15} | {name:32s} | {timer.members['duration_']:<14.3e} | ")
            callback_rows.append({"callback": callback, "processor": name.rstrip('.'), "duration_": float(timer.members['duration_'])})

    print("-" * 71)

    # Print Per-Processor Duration Summary
    print("\n")
    print(" Per-Processor Duration Summary:")
    print("-" * 132)
    print(f"| {'Processor':<29} | {'Total (s)':<15} | {'Mean (s)':<15} | {'Median (s)':<15} | {'90th Percentile (s)':<19} | {'Fraction of __ALL__':<20} |")
    print("-" * 132)

    by_event_ids =  np.flatnonzero(bool_completed).tolist()

    all_total = np.sum(events['__ALL__.']['__ALL__.duration_'][bool_completed])

    for proc, branches in events.items():
        if proc == 'completed':
            continue
        process = proc.rstrip('.')
        durations = branches[f'{process}.duration_'][bool_completed].tolist()
        by_event_rows.extend(
            {"event_id": event_id, "processor": process, "duration": dur}
            for event_id, dur in zip(by_event_ids, durations)    
            )

        total = np.sum(durations)
        mean = np.mean(durations)
        median = np.median(durations) 
        percentile_90 = np.percentile(durations, 90)

        if all_total > 0:
            all_fraction = total / all_total
        else:
            all_fraction = 0
        
        print(f"| {process:<29} | {total:<15.3e} | {mean:<15.3e} | {median:<15.3e} | {percentile_90:<19.3e} | {all_fraction:<20.3e} |")
        processor_rows.append({"processor": process, "total": total, "mean": mean, "median": median, "percentile_90": percentile_90, "all_fraction": all_fraction})

        print("-" * 132)

    # Print metadata
    print("\n Metadata:")

    metadata = get_metadata(bool_completed)
    for key, val in metadata.items():
        print(f"{key}: {val}")
    
    # Write to json and csv file
    write_summary(metadata, callback_rows, processor_rows, by_event_rows)
        





