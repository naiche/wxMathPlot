INSTALLDIR ?= /usr/local
WEBDIR     ?= ../htdocs
TARGET     ?= libmathplot.a

CXXFLAGS  += `wx-config --cxxflags`
LINKFLAGS += `wx-config --libs`

SOURCES = mathplot.cpp
HEADERS = mathplot.h

.SUFFIXES: .o .cpp
.PHONY: install default clean cleanvc doxygen

OBJS = $(SOURCES:.cpp=.o)

default: $(TARGET)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXINCLUDES) $< -o $@

$(TARGET): $(HEADERS) $(OBJS)
	$(AR) r $(TARGET) $(OBJS)
	ranlib $(TARGET)

install:
	@install -v -d $(INSTALLDIR)/lib $(INSTALLDIR)/include
	@install -v $(HEADERS) $(INSTALLDIR)/include
	@install -v $(TARGET) $(INSTALLDIR)/lib

clean:
	@rm -f $(OBJS) $(TARGET) *~ *.core *.bak
	@rm -rf html dox.out dox.err
	@rm -rf Release Debug *.ncp *.opt *.plg *.ncb

doxygen:
	doxygen > dox.out

web: doxygen
	mv html/* $(WEBDIR)
	rm -rf html
	cp -f www/*.html www/*.png $(WEBDIR)


