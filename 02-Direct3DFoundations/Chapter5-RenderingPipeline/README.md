# The Rendering Pipeline

**This refers to the entire sequence of steps necessary to generate an image based on what a virtual camera sees.**

The stages of the rendering pipeline

## Input assembler (IA) Stage
This stage reads geometric data from memory and uses it to assemble geometric primitives. Geometric
primitives are generally composed of vertices and indices. The vertices can contain interesting information about the
model and create cool effects as well. Such as lighting and texture mapping.

The vertices them selves do not determine how they are rendered though. That can be determined by DirectX through the primitive
topology, See pg. 169 for all examples. As you will notice throughout graphics development there are various ways to use these for
better optimization. For example if you couple triangle lists with indices we have flexibility for clearing duplicated vertices and replacing them
with duplicated indices. The reason that we would rather have duplicated indices rather than vertices is because there is far less data behind an index
than there is a vertex.


## Vertex Shader Stage
The vertex shader will take all of the vertices that are to be rendered and output a vertex that can be rendered. In the beginning it may appear
that the vertex shader's purpose is to only put the vertex in the correct viewing space, but it can do so much more: lighting, texturing, preparation for post rendering effects.

### Local space and world space
This can be thought as taking the model from the space that you created it in, in maya or whatever your choice of 3D modeling tool, and moved it
into where you want it relative to the world that you are rendering. The world matrix also consists of the orientation and the scale of the object in 3D space.

### View space
This refers to the camera's matrix. This conveys the location of the camera within the scene in addition it tells us what the camera is looking at. This will be how we generate
our 2D image of the scene.

## Projection
This matrix will be how we turn a 3D scene into a 2D image. The first part of the projection is setting up the frustum and this requires 4 parts. These part include the
near and far planes, the angle that represents the field of view, and the aspect ratio.

## Hull Shader Stage (TBC.)
## Tessellation Stage
Tessellation allows us to subdivide the triangles of a mesh and add new triangles, these triangles can then be offset to add a new level of detail. With tesselation we get some pretty impressive benefits.
- Level of detail (LOD) - triangles near the camera are tessellated to add more detail where as those that are far away will not be rendered at such a high quality.
- Low poly count in memory
- Perform animation and physics on the low-poly model and save Tessellation for rendering

## Domain Shader Stage (TBC.)

## Geometry Shader Stage

In the geometry shader stage we can literally create and destroy geometry based on conditions.

### Stream Output Stage (TBC.)

## Rasterizer Stage

At this stage we are computing pixel colors from projected triangles.

## Pixel Shader Stage

The pixel shader is computed for each pixel fragment and can do things as simple as rendering a simple color, or as complicated as ray tracing on the gpu. Or some times even more complicated than that.

## Output Merger Stage

Every fragment that has made it passed the depth, stencil, and beyond being rejected has now graduated to this point to become a pixel and be rendered to the back buffer.
