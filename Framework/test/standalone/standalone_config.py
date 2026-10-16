from LDMX.Framework import ldmxcfg


p = ldmxcfg.Process("test")
p.max_events = 10
p.sequence = [
    ldmxcfg.processor_from_file("Standalone.cxx"),
    # a processor including headers that need Geant4 to be found
    ldmxcfg.processor_from_file("StandaloneG4.cxx", needs=["SimCore/G4User"]),
]
p.output_files = ["/dev/null"]
p.histogram_file = "standalone_histogram.root"
