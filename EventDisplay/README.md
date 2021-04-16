# Event Display for ldmx-sw

Currently, the event display is able to be used only within a container under development.
To obtain this version of the container after setting up the ldmx environment, run
```bash
ldmx-container-pull dev display
```
Since this is a new container, you are required to remove your old build and
install directories so that there is not contamination from the old container.

Now you can compile the event display program `eve`.
You must turn on the compilation of eve before trying to compile.
```
ldmx cmake -DBUILD_EVE=ON ..
```
And then you can compile like normal.

The event display takes one event file as input.
```bash
ldmx eve {events.root}
```
Use `ldmx eve --help` for more detail.
