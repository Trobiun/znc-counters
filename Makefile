MODULES_DIR = /var/lib/znc/modules

all: counters.so
	
	
counters.so: counters.cpp
	znc-buildmod $<

copy: counters.so
	cp counters.so $(MODULES_DIR)

clean:
	rm -f counters.so