from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('raw')

import sys

RAWfileName=sys.argv[1]
p.output_files = [sys.argv[2]]
log_name=p.output_files[0].replace(".root", "_toRoot.log")
n_time_samples = int(sys.argv[3]) if len(sys.argv) > 3 else 24 #default

n_ev = sys.argv[4] if len(sys.argv) > 4 else 4e6 #default

n_channels = int(sys.argv[5]) if len(sys.argv) > 5 else 16 #default
# UTC time_stamp s, timestamp clock ticks, time since spill,
# evNb, 4bit error words + 4bit empty
len_header = 4+4+4+3+1
n_words=2*n_channels*n_time_samples+len_header


from LDMX.Packing import rawio

p.sequence = [
    rawio.SingleSubsystemUnpacker(
        raw_file=RAWfileName,
        output_name="QIEstreamUp",
        detector_name="ldmx-hcal-prototype-v1.0",
        num_bytes_per_event=n_words,
    )
        ]

# single subsystem unpacker aborts all events when it runs out of data
p.max_events = int(n_ev)

p.term_log_level = 0 #1
p.log_file_name = log_name
p.file_log_level = 0
