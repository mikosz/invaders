all:
	g++ -I . -o mr222146 gtpclient.cpp mr222146.cpp gamestate.cpp invadersgtphandlers.cpp -O3 -DNDEBUG

clean:
	rm mr222146
