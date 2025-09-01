 # To compile main
main:
	g++ -std=c++11 Compression.cpp Parse.cpp main.cpp -o main.exe

# For wsl to compile main
submit:
	x86_64-w64-mingw32-g++ -std=c++11 -static Compression.cpp Parse.cpp main.cpp -o chill.exe

submitO3:
	x86_64-w64-mingw32-g++ -std=c++11 -O3 -DNDEBUG -march=haswell  -static -flto   Compression.cpp Parse.cpp main.cpp -o chill.exe

# To compile the testing main
testO3:
	g++ -std=c++11 -g -O3 -DNDEBUG -march=haswell  -static -flto Compression.cpp Parse.cpp Test.cpp TestMain.cpp -o testmain.exe

test:
	g++ -std=c++11 -g Compression.cpp Parse.cpp Test.cpp TestMain.cpp -o testmain.exe

%.o: %.cpp
	x86_64-w64-mingw32-g++ -static -std=c++11 -O3 -c $< -o $@

clean: # Remove made files
	rm *.o *.exe main 

