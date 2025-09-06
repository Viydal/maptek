 # To compile main
main:
	g++ -std=c++17 -O3 -DNDEBUG -march=haswell -flto -static Compression.cpp Parse.cpp Test.cpp main.cpp Tester.cpp -o maintest.exe

# For wsl to compile main
submit:
	x86_64-w64-mingw32-g++ -std=c++17 -O3 -DNDEBUG -march=native -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp main.cpp Tester.cpp -o chill.exe

clean: # Remove made files
	rm *.o *.exe main callgrind.*