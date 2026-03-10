## Instructions for how to do visualization with OpenGL (no GUI)


One of Geant4’s most reliable visualizers throughout the years, OpenGL
(or OGL) is a fast, powerful, yet relatively low-resource rendering
program. Despite these advantages, there are two major
disadvantages. Firstly, out-of-the-box, it only supports command-line
arguments for instructions. Perhaps in a future upgrade we can build
Geant4 with Qt enabled so we can provide at least a basic
GUI. Secondly, Geant4-specific documentation/tutorials are sparse and
have mostly been taught in-person at Geant4 workshops etc. These make
it a less attractive choice for visualization, but nevertheless, it’s
what is currently working in ldmx-sw.

### Basic visualization

`g4-vis` can be accessed with denv or with just. There is a required
argument (the `detector.gdml` file for the geometry you wish to
visualize) and an optional argument for a macro file, explained later.

As a concrete example (beginning from the directory holding this
`README`):

```
denv g4-vis ../ldmx-det-v15-8gev/detector.gdml
or
just g4-vis ../ldmx-det-v15-8gev/detector.gdml
```

The `detector.gdml` file is our gateway to all of the detector’s
sub-components. Take a look in this file, and you’ll see that it loads
(most of) the other .gdml files in this directory one by one. This is
what they call a ‘module’ implementation in GDML. This works so long
as none of the module boundaries are intersecting i.e. the modules are
not physically touching each other. You can even place modules inside
other modules, so long as the parent module fully contains the child
module (again, no boundaries crossing). We’ll cover how to examine
individual components in a later section.

You may notice when you run `g4-vis`, you get dropped at a command
line interface (CLI). To actually see the geometry you just loaded,
you’ll have to open a viewer screen:

```
/vis/open OGLIX
```

Now we have a black screen! Hooray! Let’s populate it with our loaded
geometry:

```
/vis/scene/create
/vis/scene/add/volume
/vis/sceneHandler/attach
```

The first command creates a new ‘scene’ for OGL. This is an internal
instance of a geometry. Multiple scenes can be open at once and
switched between, but this is frankly unnecessary functionality so
we’ll ignore it from here on in.

The next line adds the ‘volume’(s) i.e. the components to be
visualized. By providing no additional arguments, it loads everything
into the current scene by default. Again, we’ll cover how to load
individual components/modules later.

The final line simply connects your active scene to the viewer, using
the ‘scene handler’ intermediary. If I were designing OpenGL, viewers
and scenes would be the same thing, especially considering you can
open multiple viewers with very little overhead, and we could
eliminate two abstractions. Sadly that is not the case.

### Navigating the geometry

Navigating the geometry using a CLI isn’t ideal, but it can be
manageable, especially with a multi-screen setup. We only need a few
commands for navigation; the craft is in knowing how to use them
creatively to get the rendering you desire.

The visualization starts centered at the origin point (0,0,0), which
in most geometries is the target, and with the viewing angle set to
phi=0, theta=0. This is equivalent to standing at the downstream end
of the beamline, near the beam dump, and looking backwards along the
beam trajectory through the calorimeters. To view along the beamline
from the upstream end, rotate your viewpoint 180 degrees:

```
/vis/viewer/set/viewpointThetaPhi 180 0
```

Normally, theta specifies the zenith angle and phi specifies the
azimuth angle. However, since we use the Z axis as the beam direction
in ldmx-sw, you can consider these definitions to be effectively
swapped. You can also use a direction vector to specify your viewing
angle. The equivalent to the previous command would be

```
/vis/viewer/set/viewpointVector 0 0 -1
```

A word of warning: OGL cannot render a viewpoint directly along the
axis it considers to be ‘upward’. I never learned exactly why this is,
but I guess it’s probably doing some math behind-the-scenes while
rendering, which involves dividing by the deviation from the up
direction, resulting in a division by zero. OGL by default considers
the Y axis to be ‘up’, so these commands are undefined and will show
nothing:

```
/vis/viewer/set/viewpointVector 0 1 0
/vis/viewer/set/viewpointVector 0 -1 0
/vis/viewer/set/viewpointThetaPhi 90 90
```

It’s also possible to change the ‘focal point’ of the rendering. This
can be useful when inspecting components at different positions along
the beam path. For example, in v15, to center your viewpoint at
(roughly) the most upstream spot on the geometry, where the magnets
begin, you can use

```
/vis/viewer/set/targetPoint 0 0 -1000 mm
```

OGL doesn’t do ‘perspective’. It doesn’t matter how close or far away
a component is; its size relative to other components in the render
will stay the same. However, it’s possible to zoom in closer on the
focal point (or further away) by setting the zoom scaling factor. Try
for example”

```
/vis/viewer/zoom 0.5
```

This will zoom out, making everything effectively twice as small. You
can get the zoom back to normal with:

```
/vis/viewer/zoom 2
```

Occasionally, navigation will break, or not do quite what you want it
to do, or you’ll end up lost and unsure which component of the
geometry you’re even looking at. You can always return to the default
view (focal point at (0,0,0), zoom 1, viewing angle (0,0)) with:

```
/vis/viewer/reset
```

When you’re done with your viewing, simply type ‘exit’ in the CLI to
exit.


### Loading components

There are two ways to specify which components are visualized. Either
can be useful, depending on your needs.

The simpler but maybe less elegant way is to edit the .gdml files
directly, especially `detector.gdml`. If you delete or comment out one
of the components, it’s removed from the geometry. You can see how to
make GDML comment blocks in the files:

```
<!--This is an example GDML-style comment and will be ignored when
loading the geometry→
```

Another word of warning: removing components for visualization
purposes will break ldmx-sw in other ways. For example, if you remove
the `hcal.gdml` module from `detector.gdml` so that you can see the
other volumes more clearly, then try to run ldmx-sw, it won’t be able
to load an HCal into the geometry and it will crash.

You can also load individual volumes by specifying their names in the
OGL CLI (recommended). By default, a loaded volume will also display
all ‘nested’ volumes, i.e. volumes fully contained inside it (in
Geant4 geometry terms, we refer to the larger volume as the ‘parent’
and the nested volumes as the ‘daughters’). Each of the modules in
`detector.gdml` contains one big parent volume, which we often refer
to as the envelope (in case you needed more jargon in your life), so
by specifying this parent volume, you can load individual modules, or
combinations thereof.

With your OGL CLI open and `detector.gdml` loaded, try typing:

```
/vis/drawTree
```

This will display thousands of lines of code. Each line is the name of
one of the volumes loaded into our geometry. As an example, the last
several lines of the v15 geometry tree look like this:

```
    "ldmx_magnet_PV":0 "coil_1_vol_PV":0 "coil_2_vol_PV":0
      "shim_right_PV":0 "shim_left_PV":0
      "corner_iron_block_top_left_PV":0
      "corner_iron_block_top_right_PV":0
      "corner_iron_block_bot_left_PV":0
      "corner_iron_block_bot_right_PV":0 "center_iron_block_top_PV":0
      "center_iron_block_bot_PV":0
```

Notice that the first line has less indentation than the next
lines. The indentations indicate mother-daughter relations. Thus, all
of the lines below ldmx_magnet_PV are daughters of `ldmx_magnet_PV`,
and by default will be loaded along with it. If you’re in the v15
geometry you can try this yourself:

```
/vis/open OGLIX
/vis/scene/create
/vis/sceneHandler/attach
/vis/scene/add/volume ldmx_magnet_PV
```

Notice that we attached the scene to the viewer before even loading
anything! This is fine; the viewer will refresh each time you add
something or make other significant changes. To save you lots of
scrolling, here are the names of the envelope for each module in
`detector.gdml` as of v15:

```
tagger_PV
TS_trgt_PV
trig_scint_tagger_PV
recoil_PV
em_calorimeters_PV
hadronic_calorimeter_PV
ldmx_magnet_PV
```

The ‘_PV’ is added to all of the names automatically by GDML, and
indicates that this is a physical volume. Don’t worry too much about
it - but you do have to include it. If you `/vis/scene/add` each of
these, you’ll load the entire geometry, just like we did by providing
no arguments to this command. You can also draw the geometry tree of
individual components, for example:

```
/vis/drawTree TS_trgt_PV
```

You may notice a : followed by a number for most volume names. This is
the ‘copy number’. Often, when iteratively placing volumes, we’ll
place the same volume with the same name multiple times. The copy
numbers help them to remain logically distinct. If for some reason
you’d like to examine a single copy of a volume placed multiple times,
simply provide the copy number as a second argument when adding the
volume:

```
/vis/scene/add/volume tspad3_lightpipe_volume_PV 40
```

This loads only one of the light pipes in TSPad3, the one with copy
number 40.


### Visualization macros

So far this tutorial has covered using OGL in the CLI. It’s also
possible to write a macro - a set of commands - which can be executed
sequentially. To execute all the commands in a macro file, simply use:

```
/control/execute /path/to/macrofile.mac
```

A few example macro files have been included with this tutorial, to do
things such as examining the TSPads in greater detail, rendering the
ECal, and a secret third example which I’ll leave as a surprise.


### How to learn more

You can try your luck at googling for OpenGL tutorials. These are
often quite old, but the functionality hasn’t evolved much in the past
10 years. There are also books and stuff but I’ve never actually seen
one. I don’t know what to tell you for GDML. There’s a manual, but
it’s missing a lot of details.

There are two essential commands which can help you learn more about
OpenGL: `ls` and `help`. First, let’s re-examine a typical OGL
command:

```
/vis/viewer/set/targetPoint
```

You can think of `/vis/`, `/viewer/`, and `/set/` as directories. This
is just a way of organizing the hundreds of OGL commands
logically. Like a typical directory, you can use `ls` to see what’s
inside of it:

```
ls /vis/viewer/set
ls /vis
```
etc

However, the last part, `targetPoint`, is not a file. It is still a
command, and almost all commands accept arguments. To learn more about
the functionality and arguments of the command, use `help`.

```
help /vis/viewer/set/targetPoint
```
etc


### Technical details that don’t really fit in the rest of the tutorial
### but should be written somewhere

A recent PR has ‘fixed’ OGL in ldmx-sw by moving the XSD style sheets
- used by GDML for XML-style formatting - from online to offline
(i.e. local) locations. It seems that for whatever reason, internet
functionality is limited in the current version of ldmx-sw. I wouldn’t
even know where to begin fixing this, so we accrue a bit more tech
debt and work around it. For now, I’ve taken advantage of an existing
piece of the CMakeLists.txt for detectors, which gives absolute paths
to all of the GDML files when compiling ldmx-sw, and used it for the
XSD files as well. This has one major disadvantage in that it requires
the same set of XSD files to exist as copies in each geometry version
directory. So there will be a `/path/to/v14/*.xsd` and a
`/path/to/v15/*.xsd` and so on. This can be solved in a future small
PR, in which the .xsd files are in a separate directory and are only
copied once for all geometry versions.


### Glossary

**Copy** - One instance of one volume which has been placed multiple
times. For example, a TSPad has 48 identical scintillator bars, so
there are 48 copies of the volume for a scintillator bar.

**Daughter** - A volume entirely surrounded by a larger volume, its
‘parent’.

**Envelope** - The parent volume of an entire module, containing all its
components.

**Focal point/focus** - The central point of the OpenGL
visualization. Defaults to the origin.

**GDML** - Geometry Description Markup Language, a variant of XML used to
describe geometries. Like XML, it’s basically an enormous set of
nested parameters, which are logically read and assigned to Geant4
objects during runtime.

**Geometry** - A general term for all of the volumes in a
simulation. Sometimes interchangeable with the term ‘world’, when
talking about one specific geometry.

**Module** - A self-contained collection of volumes. Has one ‘envelope’, a
giant parent volume, populated with many daughters. Load several of
these to make a complete geometry.

**OGL** - OpenGL (Open Graphics Library) is a cross-language,
multi-platform application programming interface (API) for rendering
2D and 3D vector graphics. I copied this directly from a webpage.

**Parent** - A large volume which completely encompasses one or more
smaller volumes, the daughters.

**Scene** - an internal instance of a geometry in OpenGL. A scene exists
without being displayed until it’s attached to a viewer.

**Scene Handler** - the intermediary in OpenGL which communicates between
the scene and the viewer.

**Viewer** - a graphical window in OpenGL, used to display the rendered
geometry.

**Volume** - a single object in a geometry, such as one SiPM, one HCal
quad bar etc. Interchangeable with ‘component’.

**XML** - eXtensible markup language. This is a generalized data storage
language, with tools to categorize and search for/retrieve data
quickly and unambiguously. It’s not very friendly for human
readability, and is typically generated and read automatically.

**XSD** - XML Schema Definition. This is XML’s way of defining a set of
rules for parsing and interpreting parameters. GDML is basically just
XML with special XSD files designed for geometry reading/writing.
