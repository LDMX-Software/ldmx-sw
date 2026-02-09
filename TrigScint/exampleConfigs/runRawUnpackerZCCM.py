from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('raw')

import sys

RAWfileName=sys.argv[1]
p.outputFiles = [sys.argv[2]]
log_name=p.outputFiles[0].replace(".root", "_toRoot.log")
if len(sys.argv) > 3 :
    p.maxEvents = int(sys.argv[3])
else :
    p.maxEvents = 4_000_000
print(f"Processing {p.maxEvents} events")

from LDMX.Packing import rawio

p.sequence = [
    rawio.SingleSubsystemUnpacker(
        dat_file = RAWfileName,
        output_name = "ZCCMoutput",
        frame_offset = 1,
        subsystem = 'ts',
    )
]

p.logger.termLevel = 0 #1
p.logger.file_name = log_name
p.logger.fileLevel = 0

