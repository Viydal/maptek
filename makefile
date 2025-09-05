 # To compile main
main:
	g++ -std=c++11 -O3 -DNDEBUG -march=haswell -flto -static Compression.cpp Parse.cpp Test.cpp main.cpp -o main.exe

# For wsl to compile main
submit:
	x86_64-w64-mingw32-g++ -std=c++11 -O3 -DNDEBUG -march=native -flto -fno-exceptions -fno-rtti -static Compression.cpp Parse.cpp main.cpp -o chill.exe

# To compile the testing main
test:
	g++ -std=c++11 -O3 -DNDEBUG -march=haswell -flto -static Compression.cpp Parse.cpp Test.cpp TestMain.cpp -o testmain.exe

clean: # Remove made files
	rm *.o *.exe main callgrind.*