set -xe

pushd ..
source external/emsdk/emsdk_env.sh
mkdir -p build-web
pushd build-web
emcmake cmake .. -DPLATFORM=Web -DBUILD_EXAMPLES=OFF
emmake make -j32
python3 ../web/server.py
popd
popd
