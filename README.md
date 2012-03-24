README
===

Load81 is an attempt at creating a Codea-inspired environment to teach
children how to write Lua programs. It features a graphical programming
environment and a Commodore-64 style integrated editor so that the programmer
is confined into a friendly environment with a simple editor:

![Load81 Editor](http://antirez.com/misc/codakido_screenshot_1.png)

The following is a screenshot of the running program (examples/asteroids.lua).
The programmer can currently switch between edit and play mode pressing the
ESC key.

![Load81 Asteroids](http://antirez.com/misc/codakido_screenshot_3.png)

Load81 is written in ANSI C and uses `SDL` and `SDL_gfx` and `SDL_image`, so
should compile on Mac OS X and Linux without issues. It should not be hard
to port it to Windows.

The coordinate system and the basic drawing functions are compatible with
Codea (check http://twolivesleft.com/Codea/ for more information), but there
is no support for stroke.

There is no aim at Codea compatibility, but who is familiar with Codea should
feel at home with Load81 in terms of API and structure of the program.

I wrote it mainly because I and my children have fun with Codea but we don't
have an iPad at home, and using a real keyboard sometimes can be less
frustrating.

The name Load81 originates from the fact that in popular Commodore home
computers the command `LOAD "*",8,1` would load the first program on the disk
starting from the file-specified memory location.

USAGE
===

Start Load81 with:

    ./load81 example.lua

To switch between program and editor mode press the ESC key.

Check the "examples" folder for small examples.

PROGRAMMING INTERFACE
===

Drawing functions:

* fill(r,g,b,alpha): select the drawing color.
* filled(filled): set the filled state (true or false)
* background(r,g,b): paint the whole background with the specified color.
* rect(x,y,width,height): draw a rectangle at x,y (left-bottom corner).
* ellipse(x,y,width,height): draw an ellipse centered at x,y.
* line(x1,y1,x2,y2): draw a line from x1,y1 to x2,y2.
* text(x,y,string): print the specified text at x,y using a bitmap font.
* triangle(x1,y1,x2,y2,x3,y3): draw a triangle with the specified vertex.
* getpixel(x,y): return the red,gree,blue value of the specified pixel.
* polygon(xv, yv): draw a polygon using a table of X values and a table of Y values.

Sprite functions:

* sprite(file,[x,y,[rotation],[antialiasing]]): draw sprite at coordinates with the specified rotation (in degrees, default 0) and antialiasing (default false).

Returns a sprite userdata object, with the following functions

* getHeight(): returns the height of the sprite.
* getWidth(): returns the height of the sprite.
* getTiles(): returns x,y for the number of tiles horizontally and vertically.
* setTiles(x,y): set the number of tiles horizontally and vertically.
* getTileSize(): return w,h for the size of a tile, calculated from the width and height of the image divided by the number of tiles horizontally and vertically.
* getTileNum(): returns the number of tiles.
* tile(x,y,tileNum,[rotation],[antialiasing]): draw a tile using tileNum at coordinates with the specified rotation (in degrees, default 0) and antialiasing (default: false).
* draw(x,y,[rotation],[antialiasing]): draw sprite at coordinates with the specified rotation (in degrees, default 0) and antialiasing (default: false).

Control functions:

* setFPS(fps): Set the frame rate. For default it's set to 30 frames per second.

KEYBOARD EVENTS
===

To check if a key 'a' is pressed use:

    if keyboard.pressed['a'] then ...

SDL Key symbol names are used. You can easily find how a given key is
called using the following Lua program:

    function draw()
        for k,v in pairs(keyboard.pressed) do
            print(k)
        end
    end

(You can find this program under the examples folder).

LOW LEVEL KEYBOARD EVENTS
===

It is also possible to trap low level SDL events accessing keyboard.state
and keyboard.key fields of the keyboard table.

keyboard.state is one of:

    "down" -> KEYDOWN event
    "up"   -> KEYUP event
    "none" -> No event

keyboard.key is set to the key pressed or released when state is different
than "none".

MOUSE EVENTS
===

mouse.x and mouse.y gives you the current mouse coordinates. To check
if a button is pressed use:

    if mouse.pressed['1'] then ...

Mouse buttons are called '1', '2', '3', ... and so forth.

LICENSE
===

Load81 was written by Salvatore Sanfilippo and is released under the
BSD two-clause license, see the COPYING file for more information.
