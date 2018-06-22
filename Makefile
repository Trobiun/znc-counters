MODULES_DIR = /var/lib/znc/modules
export INCLUDES=-Ilib

all: counters.so
	

counters.so: counters.cpp
	znc-buildmod $?

copy: counters.so
	cp $< $(MODULES_DIR)

.PHONY: clean
clean:
	rm -f counters.so