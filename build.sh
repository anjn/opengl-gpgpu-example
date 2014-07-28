# linux
# g++ -g -o main main.cpp -lglut -lGLEW

# mac
g++ -g -c -o main.o main.cpp -I/opt/X11/include
g++ -g -c -o glsl.o glsl.cpp -I/opt/X11/include

g++ -o main main.o glsl.o -L/opt/X11/lib -lgl -lglu -lglut -lGLEW
