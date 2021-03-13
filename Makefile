all:
	cd lib && $(MAKE)
	cd app && $(MAKE)

clean:
	cd lib && $(MAKE) clean
	cd app && $(MAKE) clean
