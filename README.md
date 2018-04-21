## LD 41 base code

wpl is my platform layer; it mostly works; I was in the middle of adding audio support; the todo looks like this:

- Audio (wasapi for win32, integrate existing mixer with SDL2, ??? for emscripten)
- Fonts (I'll be using ChevyRay's pixel art fonts, with a system borrowed from my other project, wiggle)
- Texture Atlas (concatenating a bunch of textures together making the rendering setup much simpler)

None of those are going to be done before the jam, so I'm packaging the non-public files here just in case. 

For now let's just say it's under the MIT license. More final releases will be more comprehensively licensed. 
