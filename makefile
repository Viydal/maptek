# To compile main
main:
	g++ -std=c++17 -O3 -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp main.cpp Tester.cpp -o main.exe -g
	
#test build with debug info becuase i was too lazy to set up args
mainTest:
	g++ -std=c++17 -fopenmp -ffast-math -O3 -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp mainTest.cpp Tester.cpp -o mainTest.exe -g -fexceptions

# To test code in a windows environment
windowsTest:
	x86_64-w64-mingw32-g++-posix -std=c++17 -O3 -DNDEBUG -march=native -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp mainTest.cpp Tester.cpp -o mainTest.exe

# For wsl to compile main
submit:
	x86_64-w64-mingw32-g++ -ffast-math -fopenmp -std=c++17 -O3 -DNDEBUG -march=native -flto -static -fno-exceptions -fno-rtti Compression.cpp Parse.cpp Test.cpp main.cpp Tester.cpp -o chill.exe

test:
	g++ testGenerator.cpp Parse.cpp Test.cpp -o MakeTests.exe

clean: # Remove made files
	rm *.exe main callgrind.*