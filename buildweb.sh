set -xe

mkdir -p build-web
pushd build-web
emcmake cmake .. -DPLATFORM=Web -DBUILD_EXAMPLES=OFF
emmake make -j32
popd
python3 server.py