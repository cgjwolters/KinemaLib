
help:
	@echo
	@echo "Targets:"
	@echo "  <Empty> or all : Builds the libraries (release and debug)"
	@echo "  clean		 : Removes all object files"
	@echo "  distclean	 : Removes all object files and the libraries"
	@echo "  crlf		 : Removes carriage return characters from all source and makefiles"
	@echo
	@exit;

ifndef undefclean
.phony: clean distclean

clean:
	rm -f $(OBJS) $(OBJS:.o=d.o) $(OBJS:.o=.d); \
	exit;

distclean:
	rm -f $(OBJS) $(OBJS:.o=d.o) $(OBJS:.o=.d) $(LIB) $(LIBD); \
	exit;
endif

.phony: crlf help

$(LIB) : CXXFLAGS += -O2
$(LIB) :: $(OBJS)
	$(AR) -r $@ $^

$(LIBD) : CXXFLAGS += -g
$(LIBD) :: $(OBJS:.o=d.o)
	$(AR) -r $@ $^

$(LIB) :: $(OBJS:.o=.d)

crlf: 
	@find . \( -name Makefile -o -name '*.cpp' -o -name '*.c' -o -name '*.h' \) \
	-exec /bin/sh -c "mv {} {}.$$$$; sed 's/\r//g' {}.$$$$ > {}; rm -f {}.$$$$; chmod 664 {}" \;
	
%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

%d.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

%.d: %.cpp
	@set -e; rm -f $@; \
	$(CXX) -MM $(CPPFLAGS) -MT "$@ $(@:.d=.o) $(@:.d=d.o)" $< > $@

ifdef $OBJS
-include $(OBJS:.o=.d)
endif
