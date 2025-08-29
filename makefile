main: Compression-main.o Parse-main.o main-main.o # To compile main
	g++ Compression.o Parse.o main.o -o main.exe

submit: main-submit.o Compression-submit.o Parse-submit.o # For wsl to compile main
	x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o chill.exe main.o Compression.o Parse.o

test: TestMain-test.o Compression-main.o Parse-main.o Test-test.o
	g++ Compression.o Parse.o TestMain.o Test.o -o testmain.exe

# Functions for main script
Compression-main.o: Compression.cpp
	g++ Compression.cpp -c

Parse-main.o: Parse.cpp
	g++ Parse.cpp -c

main-main.o: main.cpp
	g++ main.cpp -c

# Functions for submit script
Compression-submit.o: Compression.cpp
	x86_64-w64-mingw32-g++ Compression.cpp -c

Parse-submit.o: Parse.cpp
	x86_64-w64-mingw32-g++ Parse.cpp -c

main-submit.o: main.cpp
	x86_64-w64-mingw32-g++ main.cpp -c

# Functions for test script
TestMain-test.o: TestMain.cpp
	g++ TestMain.cpp -c

Test-test.o: Test.cpp
	g++ Test.cpp -c

clean: # Remove made files
	rm *.o *.exe main