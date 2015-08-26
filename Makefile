all:
	g++ -o a.out main.cpp gl_staff.cpp -O3 -std=c++11 -lpthread -lgomp -lopencv_core -lopencv_highgui -lglut -lX11 -lGL # GLX
clean:
	rm -f a.out
