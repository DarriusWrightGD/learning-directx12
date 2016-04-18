# DirectX Initialization

## COM (Component Object Model) is what allows DirectX to be programming language independent, and backwards compatible.

There are three main functions to understand about COM objects.

1. Get - returns the pointer behind the COM interface.
2. GetAddressOf - returns the address of the pointer behind the COM interface.
2. Reset - sets the comptr instance to nullptr.

## The Swap Chain and Page Flipping

In this you have a front and back buffer that will be used to display the frames that are rendered. Initially you will start with a cleared front buffer, then begin drawing information into the back buffer once this is complete the back buffer will switch with the front and the process will continue as so. This is effectively an effortless task since it is simply a pointer swap. The act of swapping these buffers is known as presenting.

## Depth buffering

The depth buffer does not contain image data, but rather information about the pixel itself. The depth buffer contains values for each pixel ranging from 0.0 to 1.0, closest to furthest. This technique allows us to represent depth in our renderings. Therefore when things get rendered the only way that something will change the color of a pixel is if that object is closer (At least until alpha gets involved). This is also known as z buffering.

## Resources and descriptors

Resources are what the GPU will read and write to, and descriptors are how we describe the data that the resource holds and how it will be used.

There are a few different type of descriptors in the DirectX 12 API.

Constant buffers (CBV), Shader Resources (SRV), and unordered access view resources (UAV).
Sampler descriptors are for sampler resources
RTV descriptors are for render target resources 
DSV descriptors describe depth/stencil resources

A descriptor heap is an array of descriptors: the memory backing for all the descriptors of a  type that the application uses. Therefore there needs to be a separate heap for each type. There can be multiple of the same type.

Multiple descriptors can also reference the same resource, i.e. referencing different subregions of a resource or using a render target as a shader resource.

## Multisampling theory

Aliasing occurs because pixels are not infinitely small, and therefore when rendering things like lines or triangles we generate a stair effect. To get around this we use different multisampling techniques. One is known as supersampling, where the resolutions is made 4x larger, rendered, then when it is time to present downsampled and averaged. Multisampling makes the back buffer and depth buffer larger, but computes at the pixel’s center once instead of in each sub pixel.

* For more information about creating a sample description and querying the number of quality levels for a given texture format see pages 99-100.

Feature Levels are important to those looking to support wider audiences to get a specific feature level use the enum D3D_FEATURE_LEVEL_ followed by the feature number i.e. 11_1

## Residency

This awesome feature allows you to load and unload resources when unnecessary. For example changing levels. For manual control use the following functions on the device: MakeResident and Evict, both take in the number objects and a pageable object. Check page 107 for more details.

## CPU/GPU Interaction

With graphics programming we will obviously have to deal with two processors the CPU and the GPU and there are times that they will need synchronized. To get the best performance we want keep both busy and minimize the amount of synchronizations. The reason that synchronizations are unwanted because this causes one processing unit to become idle while waiting for the other to finish work, ruining the parallelism.

## The Command Queue and Lists

The GPU contains a command queue, and through command lists the the CPU will send commands that it wants done. These command will wait in the GPU’s command queue until ready to be processed. Essentially you don’t want either of these to get too empty or full as this will cause either the GPU or CPU to become idle. What you want is for both the GPU and CPU to be busy.

See pages 108-111 for implementation details.

Essentially you create a command queue, then create command lists for the command queue to execute. Once these have been created you can add commands such as setting viewport, or drawing we close the command list before passing it to the command queue to be executed. An important thing to remember about command lists is that they will need to have memory that backs them, this is done through a command allocator which will need to be reset once per frame once the GPU has finished rendering. Which is where GPU/CPU Sync comes into play.

## CPU/GPU Synchronization

There comes a time when you need to wait until after something has occurred on the CPU before drawing on the GPU. i.e. update a position. In this case you will need to set up a fence that will wait until the GPU is done processing it’s command, such as rendering, before continuing with the CPU. The way that we do this is through a fence, which will essentially wait until an event has fired marking that all the commands that we requested be executed have been executed.
