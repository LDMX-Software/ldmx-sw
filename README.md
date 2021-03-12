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

Here is a quick reference table for moving the camerage around.

Command | Description
--------|------------
`/vis/viewer/pan x y units` | Move camera a certain amount left/right (x) and up/down (y) a given amount of units
`/vis/viewer/set/viewpointThetaPhi theta phi units` | Rotate the camerage to the input direction vector in degrees or radians
`/vis/viewer/zoom scale` | Zoom towards the center of the image by the input factor

#### Limitations
The class we use to read-in our GDML and construct the Geant4 detector model
makes a large number of assumptions about how the GDML is structured.
With this in mind, it is best to use the central `detector.gdml` file as 
the "entrypoint" and comment out the different parts of the detector
you don't want to see.

#### Tips and Tricks
- The central `detector.gdml` file asks for other GDML files,
  so it is best to run this executable _in the same directory_ as the detector
  description you want to visualize.

- You can make multi-line comments in GDML by using `<!-- ... -->`.
  For example, the magnetic field is not part of the visualization, but
  is loaded when constructing the detector. You can ignore this by
  commenting out the magnetic field part of `detector.gdml`.
