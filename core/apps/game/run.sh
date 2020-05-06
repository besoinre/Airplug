#!/bin/bash
mkfifo /tmp/fifo1 /tmp/fifo2
./game  < /tmp/fifo1 > /tmp/fifo2 &
./game  < /tmp/fifo2 > /tmp/fifo1 &
echo "start" > /tmp/fifo1
