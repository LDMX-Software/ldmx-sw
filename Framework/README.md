# Framework

A event-by-event processing Framework using CERN's [ROOT](https://github.com/root-project/root) and C++17.

The core idea of this Framework is the assumption that our data (simulated or real) can be grouped into "events" that we can assume are independent (for the software, not necessarily true in real life).
Each event is given to a sequence of "processors" that can look at the current data in the event and potentially produce more data to put into the event.
The object that carries the event data between processors in the sequence is known as an "event bus".
The objects on the "event bus" are "passengers" that each carry something for the processors to look at.

The behavior of this Framework is dynamically configured at run-time by running an input python script and then translating python objects into their C++ counter-parts.
This configuration style is extermely flexible and allows both C++ and Python to do what they do best.

Besides this core functionality of processors looking at data event-by-event, there are additional helpers that allow processors to log through boost logging and store data in a less-hierarchical format (Ntuples).
