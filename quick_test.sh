cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug
cmake --build build
cd build
ctest -V --output-on-failure