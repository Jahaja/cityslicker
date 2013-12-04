## Cityslicker

A TCP server (epoll based) in C for searching geonames locations within a bounding box.
It will return the geoname-id of the cities within the requested bounding box. The result is sorted on population, largest first.

**NOTE**: This is an educational project to further familiarize myself with C and a few concepts within it (namely epoll based socket server). Use at your own risk.

#### Usage
1. Download/Clone the files and navigate to the `src/` directory.
2. `make`
3. Download and unzip the geonames dump from http://download.geonames.org/export/dump/
4. Start the server using `./cs [geonames-file]`

For client, see the python example at `client/cityslicker.py`.

#### How it works

Cityslicker works by parsing and loading all cities from the geonames file into an array and then sorting the array on latitude. When a request with with a bounding-box (south-west coordinates and north-east coordinates) is received it will use a binary-search to find the cities within the received latitude range and subsequently make sure the results are within the longitude range as well. This is currently done by just iterating through the results and filtering out results outside the longitude range - thus a sequential search.

##### Further improvements 

* Return full results rather than just geoname-ids.
* Use a well-known serialization format such as msgpack.
* Create an index for longitude as well and do an intersection between the two results.

#### License
Released under CC0 (Public Domain Dedication).

http://creativecommons.org/publicdomain/zero/1.0/