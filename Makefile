.PHONY: build flash monitor clean menuconfig

build:
	idf.py build

flash: build
	idf.py flash

monitor:
	idf.py monitor

clean:
	idf.py clean

menuconfig:
	idf.py menuconfig
