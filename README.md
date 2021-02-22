# SimCore

This repository is focused on integrating the 
[Geant4 simulation framework](https://github.com/LDMX-Software/geant4) 
into another [event processing framework](https://github.com/LDMX-Software/Framework).

This is centered upon a processor that does the necessary
configuring, processing, and persisting steps tied with Geant4.

## Detector Visualization

The event processing framework that actually runs this simulation
is not well designed for running a visualization. For this reason,
a small program has been written that loads the input detector
description GDML file and launches an interactive Geant4 terminal.

The simulation is not well configured in this mode, so this 
interactive terminal **should only be used for visualization**.

This command is built automatically and is installed as `g4-vis`.
Run `g4-vis --help` for an explanation on how to use this executable.

#### Common Geant4 Vis Commands
More detailed documentation for these commands are given in the 
[Geant4 Book for App Developers](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Visualization/commandcontrol.html#scene-scene-handler-and-viewer)

In general, you can get started quickly by running the following
commands in the given sequence after the Geant4 interactive terminal
is launched.

```
/vis/open OGL
/vis/drawVolume
/vis/viewer/refresh
```

You will likely want to change the point of view from which you are looking at the
detector. The [camera working commands](https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Visualization/commandcontrol.html#basic-camera-workings-vis-viewer-commands)
is what Geant4 calls these.

