.cpp.o:
	g++ -c -o $@ $<

tuiobleep: TuioApp.o TuioClient.o main.o
	gcc -o $@ $+ -ljack -loscpack -lm

clean:
	rm *.o tuiobleep
