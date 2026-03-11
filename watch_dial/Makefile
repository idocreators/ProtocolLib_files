CXX = g++
CXXFLAGS = -std=c++11 -Wall -DLIBDEFLATE_DLL
PKG_CONFIG_PATH = /ucrt64/lib/pkgconfig
INCLUDES = -IC:/msys64/ucrt64/include/gtk-3.0 \
           -IC:/msys64/ucrt64/include/pango-1.0 \
           -IC:/msys64/ucrt64/include/harfbuzz \
           -IC:/msys64/ucrt64/include/fribidi \
           -IC:/msys64/ucrt64/include/atk-1.0 \
           -IC:/msys64/ucrt64/include/cairo \
           -IC:/msys64/ucrt64/include/freetype2 \
           -IC:/msys64/ucrt64/include/pixman-1 \
           -IC:/msys64/ucrt64/include/gdk-pixbuf-2.0 \
           -IC:/msys64/ucrt64/include/libpng16 \
           -IC:/msys64/ucrt64/include/webp \
           -IC:/msys64/ucrt64/include/json-glib-1.0 \
           -IC:/msys64/ucrt64/include/glib-2.0 \
           -IC:/msys64/ucrt64/lib/glib-2.0/include

LIBS = -LC:/msys64/ucrt64/lib \
       -lgtk-3 -lgdk-3 -ljson-glib-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0 \
       -lpango-1.0 -lharfbuzz -latk-1.0 -lcairo -lgdk_pixbuf-2.0

SOURCES = main.cpp mkWatchFace.cpp
HEADERS = mkWatchFace.h
OBJECTS = main.o mkWatchFace.o
TARGET = mkWatchFace.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

main.o: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c main.cpp -o main.o

mkWatchFace.o: mkWatchFace.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c mkWatchFace.cpp -o mkWatchFace.o

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean