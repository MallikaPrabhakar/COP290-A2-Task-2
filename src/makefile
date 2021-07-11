objects = font.o map.o simulation.o sim.o main.o
LIBS = -lSDL2_image -lSDL2_ttf -lSDL2_mixer
CFLAGS = `sdl2-config --cflags`

run: install
	./simulation

install: simulation

simulation: $(objects)
	g++ -g $(objects) -o simulation `sdl2-config --libs` $(LIBS)

main.o: main.cpp map.o simulation.o
	g++ -c $(CFLAGS) main.cpp

map.o: map.cpp map.hpp
	g++ -c $(CFLAGS) map.cpp

simulation.o: simulation.cpp simulation.hpp map.o font.o
	g++ -c $(CFLAGS) simulation.cpp

sim.o: sim.cpp sim.hpp simulation.hpp map.o font.o
	g++ -c $(CFLAGS) sim.cpp

font.o: font.cpp font.hpp
	g++ -c $(CFLAGS) font.cpp

clean:
	rm -f $(objects) simulation

reinstall: clean install
