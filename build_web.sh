set -xe

source external/emsdk/emsdk_env.sh
mkdir -p build-web
pushd build-web
emcmake cmake .. -DPLATFORM=Web -DBUILD_EXAMPLES=OFF
emmake make -j32
popd
python3 server.py