 # To compile main
main:
	g++ -g -std=c++17 -O3 -march=haswell -flto -static Compression.cpp Parse.cpp Test.cpp main2.cpp Tester.cpp -o main.exe

# For wsl to compile main
submit:
	x86_64-w64-mingw32-g++ -std=c++17 -O3 -DNDEBUG -march=native -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp main2.cpp Tester.cpp -o chill.exe

test:
	g++ testGenerator.cpp Parse.cpp Test.cpp -o MakeTests.exe

clean: # Remove made files
	rm *.exe main callgrind.*