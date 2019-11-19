# bss
compile:

g++ -std=c++11 -O3 -o bss bss.cpp

usage examples:

./bss

./bss -c TUR

./bss -c TUR -t cable

Note: Reading and parsing countries.geojson file takes ~97% of running time. This application uses Nlohmann's single header JSON code. RapidJSON can be used for faster parsing.
