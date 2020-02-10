.phony: all

all % :
	mkdir -p lib/1.0
	mkdir -p lib/Geo/1.0
	mkdir -p InoRpm/lib
	cd Basics; $(MAKE) $@
	cd Dxf; $(MAKE) $@
	cd Dxf3D; $(MAKE) $@
	cd DxfOut; $(MAKE) $@
	cd CcdOut; $(MAKE) $@
	cd Persist; $(MAKE) $@
	cd Matrix; $(MAKE) $@
	cd cppstd; $(MAKE) $@
	cd Geo; $(MAKE) $@
	cd InoQt; qmake InoQt.pro && $(MAKE) $@
	cd InoRpm; $(MAKE) $@

help crlf:
	@$(MAKE) -s -f Makefile.inc $@
