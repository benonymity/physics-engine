
# Physics Engine
<img src="screenshot.jpg" style="max-width:400px">

This super simple physics was the snowball that resulted from an open ended data structures assignment which became a fun weekend project. I used inheritance on the shape classes, and plenty of polymorphism with virtual functions and overloaded operators, so it should count! There are still plenty of bugs with this project, mostly to do with the imprecision of floats eventually resulting in physics issues (e.g. phasing through walls/shapes/etc). Not sure what the elegant solution to that is, though there probably is one. This is all pretty rudimentary (using Euler's method) but I enjoyed learning about the ideas that are needed to create even a simple rendering of reality.

I used a nice [open-source, single-file graphics library](https://github.com/erkkah/tigr) to interface with OpenGL, but otherwise wrote this all by hand. I mostly used [Randy Gaul's physics simlulation](https://randygaul.github.io/math/2022/09/18/Game-Math-101-Writing-your-Own-2D-Math-in-CPP.html) [blog](https://code.tutsplus.com/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331t) [posts](https://randygaul.github.io/) [to learn](http://compsci.ca/v3/viewtopic.php?t=14897) about the logic behind physics simulation, but didn't just copy and paste and tried to understand all the reasoning behind everything.

If you want to compile and run this yourself, I'd recommend running `build.sh` if you're on MacOS. Otherwise, YMMV. You shouldn't need to install anything except `tigr.c` and `tigr.h`, but you will need to compile with the C++17 standard, because this uses the algorithims package added in that version of the standard library. On MacOS I can compile with:
```sh
g++ -std=c++17 main.cpp tigr.c -o physics -framework OpenGL -framework Cocoa
```
But if you're on Windows, this *should* work instead, though I haven't tested it: 
```sh
g++ -std=c++17 main.cpp tigr.c -o physics -s -lopengl32 -lgdi32
```

Enjoy!