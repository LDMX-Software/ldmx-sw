from LDMX.Framework import ldmxcfg
from LDMX.Packing import rawio
import sys

p = ldmxcfg.Process('raw')

RAWfileName = sys.argv[1]
p.output_files = [sys.argv[2]]
log_name = p.output_files[0].replace(".root", "_toRoot.log")

p.max_events = int(sys.argv[3]) if len(sys.argv) > 3 else 4_000_000
print(f"Processing {p.max_events} events")

p.sequence = [
    rawio.SingleSubsystemUnpacker(
        dat_file=RAWfileName,
        output_name="ZCCMoutput",
        frame_offset=0,
        subsystem=2,
        contributor=1,   
    )
]

p.logger.term_level = 0
p.logger.file_level = 0
p.logger.file_path = log_name