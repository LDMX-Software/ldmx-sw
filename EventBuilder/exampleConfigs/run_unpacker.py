from LDMX.Framework import ldmxcfg
p = ldmxcfg.Process('unpack')
p.run = 1
p.max_events = 100
from LDMX.Packing import rawio
import sys
p.sequence = [
  rawio.SingleSubsystemUnpacker(
      dat_file = sys.argv[1],
      frame_offset = 1,
      subsystem = 'ts',
      output_name = 'ZCCMoutput'
  )
]
p.output_files = [sys.argv[2]]
p.pause()
