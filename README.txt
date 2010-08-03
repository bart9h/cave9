cave9


ABOUT:
======

A gravity cave-exploration game by 9Hells.
It's a 3D version of the classic SF-Cave game.

http://code.google.com/p/cave9


STORY:
======

You are escaping from a bunch of enemies in your fighter aircraft,
and in a desperate move you decide to enter a large cave opening
in the mountain ahead.

The moment the ship goes into the cave, you notice that
the controls are failing, and you cannot reduce the forward speed.
You can only count on the two diagonal thrust jets to keep the ship
from colliding into the cave walls, with a faint hope that it will
lead to another opening somewhere.


INSTALL:
========

cave9 depends on the following libraries:
SDL, SDL_image, SDL_ttf, and SDL_net.

To compile it, you must have the development version of all these libraries
installed on your system (-dev on debian, -devel on fedora).

After that, just get into the cave9 src dir and run 'make'.
The binary will be placed in cave9 dir.

You also need to download the cave9_data file from the game homepage,
and extract it either under cave9 dir, or under "~/.cave9/".
It contains the font, texture and sounds.

If this doesn't work, please report to cave9@googlegroups.com

Use the --help option to check for startup options.


PLAY:
=====

Use the two diagonal thrusts of your ship to keep it from
hitting the cave walls.

The gravity will constantly push you down.
Use the right thrust to pull up and left,
and the left thrust to pull up and right.

Using both controls simultaneously will cancel
the lateral forces, and pull you straight up.


KEYBOARD CONTROLS:
==================

<Up-Arrow>  or  <Down-Arrow>  or  <W>  or  <S>
Activate both thursts.

<Left-Arrow>   or  <Left-Control>   or  <Left-Shift>  or  <A>
Activate the left thrust jet.

<Right-Arrow>  or  <Right-Control>  or  <Right-Shift>  or  <D>
Activate the right thrust jet.

<P>  or  <Space>  or  <Pause>
Pause/un-pause the game.

<F>  or  <Alt>+<Enter>
Toggles full-screen mode.

<Esc>  or  <Q>
Quits immediately.


Check the startup options with "./cave9 --help"

