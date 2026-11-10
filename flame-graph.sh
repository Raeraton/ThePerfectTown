sudo perf record -F 99 -p $(pidof app) -g -- sleep 60
sudo perf script |  ~/scripts/FlameGraph/stackcollapse-perf.pl > oot.perf-folded
~/scripts/FlameGraph/flamegraph.pl oot.perf-folded > perf.svg
