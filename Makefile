main: main.o parser.o lexer.o
	g++ -std=c++11 -o main main.o fol.cpp parser.o lexer.o

main.o: main.cpp fol.hpp
	g++ -std=c++11 -c -o main.o main.cpp

fol.o: fol.cpp fol.hpp
	g++ -std=c++11 -c -o fol.o fol.cpp

parser.o: parser.cpp fol.hpp
	g++ -std=c++11 -c -o parser.o parser.cpp

lexer.o: lexer.cpp parser.hpp
	g++ -std=c++11 -c -o lexer.o lexer.cpp

parser.cpp parser.hpp: parser.ypp
	bison -d -o parser.cpp parser.ypp

lexer.cpp: lexer.lpp
	flex -o lexer.cpp lexer.lpp

clean:
	rm -f *.o *~ parser.cpp lexer.cpp parser.hpp main
