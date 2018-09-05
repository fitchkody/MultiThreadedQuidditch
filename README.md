# MultiThreadedQuidditch

Quidditch Simulation using POSIX pthreads. This simulation is non-deterministic, so each time it's run you will see a different game.

# Compile

`make`

# Run

`./quidditch`

# Note

This needs to be compiled for multithreading. Running `make` will compile with the `-pthread` flag and run on Linux. Mac users may need to instead use `-lpthread`. Windows users, ...
