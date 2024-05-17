# AliensAndMeteors

A simple space shooter. The goal of the game is to survive as long as possible while killing as many enemies as possible.

## Highlights of the project:

Polygon-Polygon collision physics.
It made me write a reusable broadphase collision detection using a bounding volume tree hiearchy (BVH) in a similar way Box2D does it. 
Enemies can avoid meteors and are steered using boids mechanics.
I found out how to do bloom effect using shaders.

## What is bad:

Some parts are still spaghetti code because I couldn't decide what I wanted and instead of refactoring I just left it like it is because I'd rather finish this quick and make a better design on a next project.

## How to build:

Build is done standardly using CMake. 
Should work on Windows and Linux.

