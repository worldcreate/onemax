CC=gcc
TARGET=main_SGA.cpp

all:
	$(CC) $(DEBUG) $(TARGET)
rm:
	rm *.csv -f
	rm *.stackdump -f
	rm a.exe -f
run:
	./a
