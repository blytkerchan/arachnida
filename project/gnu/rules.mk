# compile rules
CPPFLAGS += -I$(srcdir)/lib

%.lo : %.c $(MAKEFILES)
	$(LIBTOOL) --mode=compile $(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

%.lo : %.cc $(MAKEFILES)
	$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	
%.lo : %.cpp $(MAKEFILES)
	$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
	
%.c : %.l
	mkdir -p `dirname $@`
	$(LEX) -d -t -B -8 $< > $@

%.c %.h : %.y
	mkdir -p `dirname $@`
	$(YACC) -td --report=all --file-prefix=$* $<
	mv $*.tab.c $*.c
	mv $*.tab.h $*.h

define LINK_LIBRARY_template
$(1) : $(2) ; $$(LIBTOOL) --mode=link $$(CXX) -o $$@ $$^ $$(LDFLAGS)
endef

define LINK_BINARY_template
$(1) : $(2) ; $$(LIBTOOL) --mode=link $$(CXX) -o $$@ $$^ $$(LDFLAGS) $(3)
endef
