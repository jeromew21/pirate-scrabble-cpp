# Pirate Scrabble CPP [Title TBD]
This is a frontend implementation of Pirate Scrabble written in pure C++.

Goals:
- Can run on 4 major platforms: Windows, Mac, Linux, Web
- Fast startup and load times
- Fast iteration
- Fast compilation

## Building
Non vendored dependencies:

```
cd external
git clone https://github.com/jeromew21/frameflow
```

### Linux
Should work out of the box, with the exception of a handful of development libraries that are 
distro-specific.

### Apple
Should work out of the box.

### Windows (MSVC)
Set up vcpkg
```
cd external
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
vcpkg.exe install ixwebsocket[mbedtls] zlib
```

Then build with MSVC toolchain:
```
# Something like...
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```
This script is collected and most up-to-date in `win32/build_win.bat`.

```
cd win32
build_win.bat
```

### Web (Emscripten)
Set up emscripten:
```
cd external
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

Then build with emscripten toolchain.
This script is collected and most up-to-date in `web/build_web.sh`.

```
cd web
./build_web.sh
```