main: Compression-main.o Parse-main.o main-main.o # To compile main
	g++ Compression.o Parse.o main.o -o main.exe

submit: main-submit.o Compression-submit.o Parse-submit.o # For wsl to compile main
	x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o chill.exe main.o Compression.o Parse.o

Compression-main.o: Compression.cpp
	g++ Compression.cpp -c

Parse-main.o: Parse.cpp
	g++ Parse.cpp -c

main-main.o: main.cpp
	g++ main.cpp -c

Compression-submit.o: Compression.cpp
	x86_64-w64-mingw32-g++ Compression.cpp -c

Parse-submit.o: Parse.cpp
	x86_64-w64-mingw32-g++ Parse.cpp -c

main-submit.o: main.cpp
	x86_64-w64-mingw32-g++ main.cpp -c

clean: # Remove made files
	rm *.o *.exe
