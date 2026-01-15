# Pirate Scrabble CPP [Title TBD]
This is a frontend implementation of Pirate Scrabble written in pure C++.

Goals:
- Can run on 4 major platforms: Windows, Mac, Linux, Web
- Fast startup and load times
- Fast iteration

## Building

Non vendored dependencies: Drop frameflow and emsdk in `./external`

### Desktop
```
mkdir build
cmake -S . -B build
cmake --build build -j8
```

### Web
```
mkdir -p build-web
cd build-web
emcmake cmake .. -DPLATFORM=Web -DBUILD_EXAMPLES=OFF
emmake make

cd ..
python3 server.py
```