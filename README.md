QuadrantTracker
===============
Color-based object tracking

This is a simple color tracking program. The user clicks on or drags a box around a desired target, and the program tracks the
target, determines its coordinates, and redraws the box on each frame. Each frame is divided into four quadrants. The quadrant
that a detected target is in will become highlighted, and if the target is centered in the center square, all quadrants will
become highlighted. If no targets are detected, no quadrants will be highlighted (see screenshots).

I hope to eventually expand this code and apply it to computer vision assited quadrocopters. The concept of dividing the image
into quadrants is intended to help with quadrocopter navigation: it'll tell the quadrocopter which direction to move in in
order to keep the tracked target centered.
