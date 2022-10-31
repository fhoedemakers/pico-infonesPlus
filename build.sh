cd `dirname $0`
[ -d build ] || mkdir build
cd build || exit 1
cmake .. -DCMAKE_BUILD_TYPE=RELEASENODEBUG ..
#/usr/bin/cmake --build /home/pi/projects/pico/pico-infones/build --config Debug -j 6 --
#/usr/bin/cmake --build /home/pi/projects/pico/pico-infones/build --config Debug --target clean -j 6 --
#make -j 4
