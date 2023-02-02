mkdir -p build
cd build

CC=$(which mpicc) CXX=$(which mpicxx) \
cmake \
-DCMAKE_BUILD_TYPE=DEBUG \
-DMALLOB_USE_JEMALLOC=1 \
-DMALLOB_LOG_VERBOSITY=4 \
-DMALLOB_ASSERT=1 \
-DMALLOB_SUBPROC_DISPATCH_PATH=\"build/\" ..

make; cd ..