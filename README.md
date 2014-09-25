Ball Rotation Using Quternion
====================

Part 1 Rendering a textured sphere

In this part, I build a OpenGL 2.1+ program with shaders to render a textured sphere in Linux platform. Instead of using glutSolidSphere, I write a similar function to create the sphere vertices, and then use SOIL (simple opengl image library) for loading texture. Figure 1 shows the results.

Part 2 Rotating the sphere

Here I implement a virtual trackball interface using quaternions for rotating the sphere. The quaternions is more efficient than rotation matrices, and importantly, it is in a form that can be interpolated as well as used in compiling a series of rotations into a single representation. Figure 2 shows the rotation result.
