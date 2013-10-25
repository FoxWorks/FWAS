FoxWorks Aerospace Simulator
--------------------------------------------------------------------------------
FWAS is an open-source aerospace simulator under development. It is based around
the EVDS physics simulation library.

The simulators main goal is prediction of aircraft/spacecraft flight characteristics,
simulation of their flight in a wide operating range, including 30 km - 120 km region and hypersonic flight.

The physics model for FWAS is based on blade element theory (quasi-3D wing theory)
and CFD/experiment derived aerodynamic coefficients. The final flight model is combined
from two approaches, steady-state physics via coefficient tables and transient behavior/control
surface physics through quasi-3D wings.

It is also possible to use it as a conventional flight simulator for existing aircraft
(the procedural approach to creating aircraft models is similar to X-Plane flight simulator).


Features
--------------------------------------------------------------------------------
 - Physics defined by geometric parameters. Spacecraft/aircraft fly as they look.
 - Powerful vessel designer/editor (FoxWorks Editor, see below)
 - Aerodynamics and rigid body physics for rockets and aircraft
 - Can use built-in visualization or an existing rendering engine, such as EDGE, X-Plane 9 or 10.
 - First-order simulation of reentry heating, detailed atmospheric model, support for adding additional physics models
 - Flexible way to define simulation scenarios - an entire solar system can be modelled, or just the planet Earth. The simulation can be initialized with any state
 
 
Development Information
--------------------------------------------------------------------------------
FWAS can be used as a standalone simulator (either without rendering or using
one of the available rendering engines), or as a part of another application
(in this case FWAS provides all the required high-level API for running the simulations).
