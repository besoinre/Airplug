Project dependencies:
  - Qt5::Core
  - Qt5::Network
  - Qt5::Widgets
  - Qt5::Gui
  - Qt5::Test
  - CMake version >= 3.1.0

To configure project ======> ./boostrap.sh
                             Go to ./build
                             Compile source code with command "make"


Applications are built in ./build/core/apps
The scripts for network construction is in ./build/core/apps/network_scripts

A possible script to run the application would be starting from path/to/Airplug :

./boostrap.sh
cd build
make
cd core/apps/network_scripts
./game_two_player.sh
