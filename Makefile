

%.config: .dummy
	./a.out custom $@
.dummy:
	true
blue:
	./a.out preset 10 4 100 10

rainbow:
	./a.out preset 0 0 0 0


.PHONY: rebuild
rebuild:
	gcc main.c $(shell pkg-config --libs --cflags libusb-1.0)
	sudo chown root:root a.out
	sudo chmod +s a.out
