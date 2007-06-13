WEBDIR     ?= ../htdocs
TEMPDIR    ?= /tmp

INSTALLDIR ?= /usr/local
TARGET     ?= lib/libmathplot.a
DEBUGFLAGS  = -g -ggdb
CXXFLAGS   += `wx-config --cxxflags`
LINKFLAGS  += `wx-config --libs`
SOURCES     = mathplot.cpp
HEADERS     = mathplot.h
VERSION     = `cat VERSION`
.SUFFIXES: .o .cpp
.PHONY: install default clean cleanvc doxygen debug

OBJS = $(SOURCES:.cpp=.o)

default: $(TARGET)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXINCLUDES) $< -o $@

$(TARGET): $(HEADERS) $(OBJS)
	$(AR) r $(TARGET) $(OBJS)
	ranlib $(TARGET)

install: $(TARGET)
	@install -v -d $(INSTALLDIR)/lib $(INSTALLDIR)/include
	@install -v $(HEADERS) $(INSTALLDIR)/include
	@install -v $(TARGET) $(INSTALLDIR)/lib

clean:
	@rm -f $(OBJS) $(TARGET) *~ *.core *.bak
	@rm -rf html dox.out dox.err
	@rm -rf Release Debug *.ncp *.opt *.plg *.ncb
	@rm -rf wxmathplot-*

doxygen:
	doxygen > dox.out

web: doxygen
	mv html/* $(WEBDIR)
	rm -rf html
	cp -f www/*.html www/*.png $(WEBDIR)

sourcerelease:
	-rm -rf $(TEMPDIR)/wxmathplot-$(VERSION)
	mkdir $(TEMPDIR)/wxmathplot-$(VERSION)
	cp -rf * $(TEMPDIR)/wxmathplot-$(VERSION)
	rm -rf $(TEMPDIR)/wxmathplot-$(VERSION)/www
	tar -c -f wxmathplot-$(VERSION).tar -C $(TEMPDIR) --exclude CVS wxmathplot-$(VERSION)
	gzip -9 wxmathplot-$(VERSION).tar
	rm -rf $(TEMPDIR)/wxmathplot-$(VERSION)

debug: $(HEADERS) $(SOURCES)
	$(CXX) -c $(CXXFLAGS) $(DEBUGFLAGS) $(CXXINCLUDES) $(SOURCES) -o $(OBJS)
	$(AR) r $(TARGET) $(OBJS)
	ranlib $(TARGET)
