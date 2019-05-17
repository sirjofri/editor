Editor
======

This program is so simple it does _not_ deserve a proper name. This editor is
a bit like `ed`, but more simple with only basic editing functions.

Functions
---------

The following list is a list of commands. Those commands do _not_ receive
parameters, if they need one (like in the file renaming command) you will be
asked.

- `l` list the file contents
- `a` append after current line
- `i` insert before current line
- `d` delete the current line
- `w` write buffer to file
- `g` print file info (name, lines, current line)
- `n` rename the file in buffer
- `.` print current line
- `[0-1]` (number) switch to this line

There is no command like `5d` or `2,4l`.

Installation and Compilation
============================

I provide a huge `Makefile` (ha ha) for easy compilation. The program is
written in C99 standard, so it should be highly portable. I also tried to only
use standard libraries. In the `Makefile` I use cflags recommended by
[suckless](https://suckless.org/).
