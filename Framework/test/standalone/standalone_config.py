from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")
p.max_events = 10
p.sequence = [
    ldmxcfg.processor_from_file("Standalone.cxx")
]
p.output_files = [ '/dev/null' ]
p.histogram_file = "standalone_histogram.root"
