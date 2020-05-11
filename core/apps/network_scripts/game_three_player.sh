#!/bin/bash

# Topology
# BAS1 <--> NET1
#             ^
#             |
#             v
# BAS2 <--> NET2
#

APPDIR="$(dirname "$PWD")"

if test -f "in*"; then
    rm in*
fi

if test -f "out*"; then
    rm out*
fi

mkfifo in1 out1
mkfifo in2 out2
mkfifo in3 out3
mkfifo in4 out4
mkfifo in5 out5
mkfifo in6 out6

# links creations
echo "Link channels"

# wait for the link creations
sleep 1

cat out1 > in2 &
cat out2 | tee in1 in4 in6 &
cat out3 > in4 &
cat out4 | tee in2 in3 &
cat out5 > in6 &
cat out6 | tee in5 in2 &

echo "init applications"
$APPDIR/game/game < in1 > out1 &
$APPDIR/game_NET/game_NET < in2 > out2 &
$APPDIR/game/game < in3 > out3 &
$APPDIR/game_NET/game_NET < in4 > out4 &
$APPDIR/game/game < in5 > out5 &
$APPDIR/game_NET/game_NET < in6 > out6 &
