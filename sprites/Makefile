
OS := $(shell uname)

ifeq ($(OS), Linux)
	GIMP=gimp
else
	GIMP=/Applications/GIMP.app/Contents/MacOS/gimp
endif


../dist/car-race.png ../dist/car-race.atlas : car.xcf car.list trade.xcf trade-font.list extract-sprites.py pack-sprites
	rm -rf car-race
	./extract-sprites.py car.list car-race --gimp='$(GIMP)'
	./extract-sprites.py trade-font.list car-race --gimp='$(GIMP)'
	./pack-sprites ../dist/car-race car-race/*
