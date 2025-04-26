# Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lpaho-mqttpp3 -lpaho-mqtt3a -lwiringPi

all: server robot

server: server.cpp
 $(CXX) $(CXXFLAGS) server.cpp -o server_app -lpaho-mqttpp3 -lpaho-mqtt3a

robot: robot.cpp
 $(CXX) $(CXXFLAGS) robot.cpp -o robot_app $(LIBS)

clean:
 rm -f server_app robot_app
