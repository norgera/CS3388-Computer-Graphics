# Bezier Pen Tool
This code is a simple implementation of a spline-based pen tool program that uses bezier curves and C++. It uses the open-source library GLFW for creating and handling windows, and basic mathematics for calculating distance between points and drawing curves.

The Bezier curve is a parametric curve used in computer graphics, design, and animation. The curve is defined by a set of control points, which can be used to adjust the shape of the curve.

![Screenshot](screenshot.png)

## Installation
In order to run the program, you will need to have GLFW installed on your machine. You can download GLFW from the official website at https://www.glfw.org/. Follow the instructions for installing the library, then run the code in your preferred C++ compiler.

## Usage
The program allows you to create Bezier curves by adding control points with your mouse. The curve will automatically be redrawn as you add more points. To add a point, simply click the left mouse button anywhere in the window. You can also click and drag an existing point to move it around.

When you add a new end point, the program will automatically create a handle that will control the shape of the curve near the point. Interior points have two handles. You can adjust the handles by clicking and dragging them. When you click and drag a handle, you will see the curve change in real time.

Press key "e" to reset spline drawing.

# Files
assignment3.cpp: The main code file for the Bezier curve drawing program.

splinedemo.mov: Demonstration of usage

