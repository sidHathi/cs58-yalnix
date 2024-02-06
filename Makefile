## Wrapper Makefile: invokes commands from src/Makefile

all:
	make -C src
	mv src/yalnix .

clean:
	make -C src clean
	rm -f yalnix

.PHONY: all clean