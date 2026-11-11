CXX ?= g++
CC ?= gcc
CXXFLAGS ?= -std=c++17 -O2 -g -Wall -Wextra -Wpedantic
CFLAGS ?= -std=c99 -O2 -g -Wall -Wextra -Wpedantic
QUIRC = vendor/quirc/lib/quirc.o vendor/quirc/lib/decode.o vendor/quirc/lib/identify.o vendor/quirc/lib/version_db.o
.PHONY: all clean test
all: QRServer QRClient QRDecode
QRServer: QRServer.cpp common.hpp
	$(CXX) $(CXXFLAGS) QRServer.cpp -o $@
QRClient: QRClient.cpp common.hpp
	$(CXX) $(CXXFLAGS) QRClient.cpp -o $@
QRDecode: QRDecode.cpp vendor/stb_image.h $(QUIRC)
	$(CXX) $(CXXFLAGS) QRDecode.cpp $(QUIRC) -lm -o $@
test: all
	python3 tests/integration.py
clean:
	rm -f QRServer QRClient QRDecode $(QUIRC)
