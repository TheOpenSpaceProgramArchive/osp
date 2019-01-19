## OSP (OpenGL + C++ Experiments)

This repo hopes to become a space exploration game along the lines of Squad's Kerbal Space Program; as of today it's a place to experiment implementing different features in C++/OpenGL.


## Features 
**(Those marked with a ✔️ are already implemented)**
**(Those marked with a ♻️ are ongoing or started)**

(The list may be expanded at any time)

##### ✔️ Keplerian Orbit Simulator for on-rails solar system
* ✔️ Simulating elliptic orbits
* ♻️ Simulating parabolic and hyperbolic orbits (Not really neccesary for now)
##### Vessel Building and Controlling
* Vessel assembly from a list of parts
    * ♻️ Procedural parts
        * ♻️ Procedural Engines
            * ✔️ Nozzle simulator
            * Liquid fueled engines
            * Solid fueled engines
        * Procedural structures (tanks, fuselage...)
    * Pre-made parts
* Vessel controlling
    * ♻️ Navball 
        * ✔️ Navball aligns with vessel
        * ♻️ Navball aligns to reference frames
    * S.A.S
##### ♻️ Planetary Surfaces
* Either very big view distance or joining together multiple scaled cameras
* Rendering of planets from far away
    * Simple billboard shader
* ♻️ Rendering of planets from the surface and near space
    * ♻️ Near space rendering
        * ♻️ Rocky bodies
            * ♻️ Cubesphere rendering
                * Very complex cubespheres for asteroids and weird-shaped bodies
                * Not spheric planets (see above, could be related)
                * Very big scale shadow rendering (Per vertex?)
            * LOD
                * Seamless LOD transitions
                * Removing seams between quads
            * Atmospheres
                * Atmospheric shader
                * Seamless transition from space to ground
                * Clouds / Cloud shadows
        * Other bodies
            * Gas body rendering
                * Clouds and animations?
##### In-Vessel physics
* Rigidbody physics simulator
* Extraction of acceleration and rotation from the simulation
* Applying forces to the vessel from outside (Gravitational gradient?)
* Seamless transition from in-vessel physics to the orbit simulator
    * Maybe they don't need to be separated, but perfomance could suffer
* Interaction with the terrain system
##### ♻️ Newtonian Orbit Simulator for vessels
* ✔️ Orbit propagation
* ✔️ Orbit predictor (threaded)
* ♻️ Maneouver planning
* Improving perfomance so many vessels can be simulated at high warp speeds
##### Modding
The engine is data-driven, so pretty much no hardcoded stuff.
Custom behaviour may be implemented using a scripting language (lua, squirrel...), or even a bigger language like C# (Mono)

## Screenshots (may be outdated)
![Current UI](READMEimg/img.PNG?raw=true "Overview of the current interface")
![DeLaval Nozzle](READMEimg/delavalnozzle.PNG?raw=true "De-Laval nozzle designed")
![Moon](READMEimg/moon.PNG?raw=true "A view of the moon with the current renderer")