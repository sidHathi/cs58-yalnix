## Wrapper Makefile: invokes commands from src/Makefile

all:
	make -C src
	mv src/yalnix .

clean:
	make -C src clean
	rm -rf yalnix

.PHONY: all clean