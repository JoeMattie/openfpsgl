# openfpsgl
FPS game in OpenGL / GLUT / OpenAL

This is a game I was working on in spring 2003

It has been hosted on sourceforge since then:

https://sourceforge.net/projects/openfpsgl/

Everything is pretty straightforward.  You will need to download and install the openAL SDK for whichever platform you intend to compile on (you can find it here: http://developer.creative.com/scripts/DC_D&H_Games-Downloads.asp?opt=2 or at www.openal.org/downloads)  I HAVE noticed a few crashing bugs/memory leaks, but nothing too showstopping.

Known bugs:

On some systems hitting the backspace key while in the console will generate an error, crashing the program.

On some systems, openAL doesn't initialize properly 100% of the time, causing a crash on startup in some cases.  Just try to run it again and it should work after a few tries.

Some texture issues and such are present when switching between fullscreen and window modes.  I don't know if there is a workaround without getting rid of GLUT entirely...

Features:

Scriptable menus, enemies, etc.
Filetypes are as follows:

levelx.enm - enemy definition file.  valid arguments are: {numEnemies, enemy, startlocation, gravity, clipping, attackdistance, speed, health, attackdamage, end, eof}

levelx.ent - describes player start location and lights.  Lights can be assigned Specular, Ambient, position values.

levelx.mat - material definition file.  lists textures to load by filename, and specifies material properties (diffuse, ambient, specular, shininess, textureID to load)  Also assignes textures to objects in the levelx.obj based on name (g or group value in the file format)

levelx.mod - model and animation definition file.  valid arguments are: {name, filenameBase, radius, numAnimations, animationName, animationFrames, animationFileNameBase, end, eof}

levelx.obj - Level mesh, .OBJ format

menus are defined in openfps.mnu. as with the .enm and .mod files anything not between double quotes is considered a comment.  valid items are: {name, option, menuitem, end, eof}
for menuitems, definitions are followed by menu item names and a consolescript command to run when clicked. if a menuitem has the name run, a menuscript command is executed and the output is drawn in place of the name.  the same with runNL except that it skips to a new line when finished.

textures are standard .raw format, sounds are .wav (although basically anything would work with openAL)  sound files are currently hardcoded into the source.

