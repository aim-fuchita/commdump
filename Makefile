all: commdump

commdump: main.o
	gcc $^ -o $@ -lbluetooth

clean:
	rm -f a.out
	rm -f *.o

