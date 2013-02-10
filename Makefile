CC = g++
RM = rm
CFLAGS = -g -Wall -std=c++11
TARGET = track-convert

$(TARGET): main.o trkconv.o tinyxml2.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o trkconv.o tinyxml2.o
main.o: main.cpp trkconv.h
	$(CC) $(CFLAGS) -c main.cpp
trkconv.o: trkconv.cpp tinyxml2.h
	$(CC) $(CFLAGS) -c trkconv.cpp
tinyxml2.o: tinyxml2.cpp
	$(CC) $(CFLAGS) -c tinyxml2.cpp

clean:
	$(RM) main.o trkconv.o tinyxml2.o