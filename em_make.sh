export srcdir="../../src"
export disabled="-Wno-pointer-sign -Wno-incompatible-pointer-types"
export sse="-msse -msse2 -msse3"
export wplflag="-DWPL_EMSCRIPTEN -DWPL_LINUX -DWPL_SDL_BACKEND"
export emflag="-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=['png'] -s SIMD=1 -s USE_WEBGL2=1 -s FULL_ES3=1"
EMCC_DEBUG=1 ASSERTIONS=2 GL_ASSERTIONS=1 GL_DEBUG=1
emcc ${disabled} ${sse} ${wplflag} -g -c ${srcdir}/wpl/wpl.c -o wpl.o ${emflag}
emcc ${disabled} ${sse} ${wplflag} -g -c ${srcdir}/main.c -o game.o ${emflag}
EMCC_DEBUG=2  emcc -Wall -g4 wpl.o game.o -s CYBERDWARF=1 -s ASSERTIONS=2 -s GL_ASSERTIONS=1 -s GL_DEBUG=1 -o game.html --preload-file assets ${emflag}
