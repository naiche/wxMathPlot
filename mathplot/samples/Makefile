TARGET     ?= mp1
CXXINCLUDE +=

CXXFLAGS  += `wx-config --cxxflags`
LINKFLAGS += -lmathplot `wx-config --libs`

SOURCES = mp1.cpp

.SUFFIXES: .o .cpp
.PHONY: default clean

OBJS = $(SOURCES:.cpp=.o)

default: $(TARGET)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXINCLUDES) $< -o $@

$(TARGET): $(HEADERS) $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LINKFLAGS)

clean:
	@rm -f $(OBJS) $(TARGET) *~ *.core *.bak *.exe
	@rm -rf Release Debug *.ncp *.opt *.plg *.ncb

