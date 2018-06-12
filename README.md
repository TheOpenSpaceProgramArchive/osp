## OSP

This repo hopes to become a space exploration game along the lines of Squad's Kerbal Space Program; as of today it's a place to experiment implementing different features in C++/OpenGL.


## Features (Those marked out are already implemented)

The features the game must have.

##### ~~ Keplerian Orbit Simulator for on-rails solar system ~~
##### Vessel Building and Controlling
The gameplay itself
##### Planetary Surfaces
The player must be able to land on planets. This requires complex rendering and physics.
##### In-Vessel physics
The game should simulate the forces that a vessel will experience, this may include full rigid-body physics.
Also vessels must be able to interact. (Docking)
##### Newtonian Orbit Simulator for vessels
A Newtonian solver seems like the best idea (Very appropiate for small objects like vessels) but time-warp is troubly. Kepler orbits may end up being used.
##### OpenGl Renering
So far just have a very basic shader and model system, but the final system would implement nice, good looking 3D graphics.
Maybe even atmospheres and similar!
##### Modding
The engine is data-driven, so pretty much no hardcoded stuff.
Custom behaviour may be implemented using a scripting language (lua, squirrel...), or even a bigger language like C# (Mono)
