# mycraft
A super-basic minecraft client
Failed to write in pure C because the glm library is written in C++ and I want to use it.
Compile: Just make it. The executable names mycraft.
Now with very basic command line packet parsing and a blank window, but hopefully the window will display enough information for players to move latter.
The IP, port and username to link with is written as constants within the code, just below the includes.
This client lacks encryption which every online minecraft server enables (according to wiki.vg).
This client enables compression of packets by default. Can be changed by modifying code.
