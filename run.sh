#!/bin/bash

export AFL_SKIP_CPUFREQ=1

screen -dmS afl-main bash -c "AFL_SKIP_CPUFREQ=1 ./afl-fuzz -i in -o out -M afl-main -- ./testbins/cyc1000"

for i in $(seq 1 31);
do
	screen -dmS afl-$i bash -c "AFL_SKIP_CPUFREQ=1 ./afl-fuzz -i in -o out -S s$i -- ./testbins/cyc1000"
done
