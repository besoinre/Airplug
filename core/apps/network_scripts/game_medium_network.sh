#!/bin/bash

# Topology
#                      BAS3
#                       ^
#                       |
#                       v
# BAS1 <--> NET1 <--> NET3 <--> NET4 <--> BAS4
#             ^                  ^
#             |                  |
#             v                  v
# BAS2 <--> NET2 <------------> NET5 <--> BAS5
#             ^                  ^
#             |                  |
#             |                  |
#             v                  v
# BAS6 <--> NET6 <-----------> NET7
#                                ^
#                                |
#                                v
#                              BAS7

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
mkfifo in7 out7
mkfifo in8 out8
mkfifo in9 out9
mkfifo in10 out10
mkfifo in11 out11
mkfifo in12 out12
mkfifo in13 out13
mkfifo in14 out14

# wait for the link creations
sleep 1

# links creations
echo "Link channels"

cat out1 > in2 &
cat out2 | tee in1 in4 in6 &

cat out3 > in4 &
cat out4 | tee in2 in3 in10 in12 &

cat out5 > in6 &
cat out6 | tee in2 in5 in8 &

cat out7 > in8 &
cat out8 | tee in7 in6 in10 &

cat out9 > in10 &
cat out10 | tee in9 in8 in4 in14 &

cat out11 > in12 &
cat out12 | tee in11 in4 in14 &

cat out13 > in14 &
cat out14 | tee in13 in12 in10 &

echo "init applications"

$APPDIR/game/game < in1 > out1 &
$APPDIR/game_NET/game_NET < in2 > out2 &

$APPDIR/game/game < in3 > out3 &
$APPDIR/game_NET/game_NET < in4 > out4 &

$APPDIR/game/game < in5 > out5 &
$APPDIR/game_NET/game_NET < in6 > out6 &

$APPDIR/game/game < in7 > out7 &
$APPDIR/game_NET/game_NET < in8 > out8 &

$APPDIR/game/game < in9 > out9 &
$APPDIR/game_NET/game_NET < in10 > out10 &

$APPDIR/game/game < in11 > out11 &
$APPDIR/game_NET/game_NET < in12 > out12 &

$APPDIR/game/game < in13 > out13 &
$APPDIR/game_NET/game_NET < in14 > out14 &
