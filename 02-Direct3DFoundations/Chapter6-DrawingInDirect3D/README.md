#Notes
- When using indices check if the model contains a larger index count than a 16 bit uint if not use it, only otherwise use a 32 bit uint.
- Since there is overhead to switching buffers global buffers may not be a bad idea. Maybe even a scene based buffer. Or even cooler. To create a seamless render flow. You could
generate the buffers based on the players position and as they head in a direction we can regenerate the buffer to only represent what is around them on a separate thread. Then load that.
