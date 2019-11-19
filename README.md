# bss
compile:

g++ -std=c++11 -O3 -o bss bss.cpp

usage examples:

./bss

./bss -c TUR

./bss -c TUR -t cable

Note: Reading countries.geojson file from filesystem takes ~97% of running time.
