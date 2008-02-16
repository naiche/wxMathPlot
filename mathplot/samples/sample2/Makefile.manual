TARGET     ?= mp2
CXXINCLUDE +=
LIBDIR    += -L../../lib/
LIBFILE   =  ../../lib/libmathplot.a

CXXFLAGS  += `wx-config --cxxflags` -g -ggdb
LINKFLAGS += -lmathplot `wx-config --libs`

SOURCES = mp2.cpp

.SUFFIXES: .o .cpp
.PHONY: default clean

OBJS = $(SOURCES:.cpp=.o)

default: $(TARGET)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXINCLUDES) $< -o $@

$(TARGET): $(HEADERS) $(OBJS) $(LIBFILE)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBDIR) $(LINKFLAGS)

clean:
	@rm -f $(OBJS) $(TARGET) *~ *.core *.bak *.exe
	@rm -rf Release Debug *.ncp *.opt *.plg *.ncb

