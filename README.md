# Prerequisites

This project needs the following packages: make, cmake, g++-13, libjsoncpp-dev, libboost-all-dev,
openssl and libssl-dev. If you have a Debian or Ubuntu Linux distribution, you can run the following
commands to install the dependencies. If you have other Linux distributions, find similar commands
to run.

```bash
sudo apt update
sudo apt install make
sudo apt install cmake
sudo apt install g++-13
sudo apt install libjsoncpp-dev
sudo apt install libboost-all-dev
sudo apt install openssl libssl-dev
```

# Build

Modify the function `compute_trade_metric` in the file `Printer.h` to make it computationally
expensive. Run the following commands to build the program.

```bash
cmake -B build
cd build
make -j 8
```

# Run

Run the following commands.

```bash
cd build
./trade_aggregate -i ../endpoints.json -o output
```

Press Control-C to stop. Alternative, run `pkill trade_aggregate` in another shell.

Note: It is possible to configure the program to query different endpoints by
modifying the config file `endpoints.json`.

Note: since the assignment asked that results be appended to an output file,
the output file is not truncated when the program is started. If this behavior
is undesirable, replace `std::ios::app` with `std::ios::trunc` in the file
`FileWriter.cpp` and recompile the program.

# Design

The program contains of multiple Scraper instances. Each instance continuously queries
an endpoint and writes the results in a lockfree queue. An Aggregator instance continuously
fetches the results from the Scrapers' queue. The Aggregator processes, filters the trades
and writes them in a lockfree queue. The FileWriter instance continuously polls the trades
from the Aggregator's queue, calls the function `compute_trade_metric` to convert trades
to string and then writes the results in a file.

The program design is Scraper(s) -> Aggregator -> FileWriter.

## Object-oriented design

Each class (Scraper, Aggregator and FileWriter) exposes a simple public API and hides
the implementation details in an internal class that is not part of the public API.
The instances use lockfree queues for communication and avoid callbacks so that they
can make progress independently and do not block each other. This design allows them
to achieve minimal latency.

# Algorithm

The Aggregator keeps track of all existing trades in a sorted lookup table (`std::map`).
The Aggregator uses the lookup table to determine whether an incoming trade is a duplicate.
If the incoming trade is not a duplicate, the Aggregator uses a built-in function
`std::map::lower_bound()` to check whether there is another trade that occurred within
5 milliseconds of the incoming trade. If this is the case, the Aggregator writes the
incoming trade in its queue. To prevent the lookup table from growing too large, the
Aggregator removes old trades when the lookup table's size reaches the limit of 1'000'000
(this limit is configurable).

# Algorithm analysis
Each trade is stored in exactly one place in the Aggregator. The space complexity is
O(N). Further, the lookup table has a configurable upperbound limit of 1'000'000.

For each incoming trade, a lookup operation is performed to check for duplicate and potentially
another lookup is performed to find trades in its 5-millisecond interval. These operations run
in O(log(N)) on the sorted lookup table (`std::map` is based on a red-black tree). Assuming that
the endpoints send K duplicates for each trade, the time complexity is O(KNlog(N)). In the
ideal case, K = 1 (no trade duplicate) and the time complexity is O(Nlog(N)).

# Tradeoff

The program consists of several components that communicate with each other via lockfree queues.
Each component runs in an independent thread. This design allows the components to make progress
independently and achieves minimal latency. However, continuous polling is less efficient than
queue locks and condition variables.

If the program runs on a CPU that has too few cores, there will be contentions among the threads.
However, this is not a problem on modern CPUs, which usually have 8 or more cores.

# Possible extensions

The program needs unit tests.

The Aggregator has the most complexity and does the most work (fetch data from Scrapers' queue,
convert string to Json and run lookup operations). This is not a problem for a few endpoints
but it won't scale for thousands of endpoints. In that case, it is possible to delegate some
of the work (fetch data and convert string to Json) to some helper instances. The new design
would be: Scraper(s) -> Helper(s) -> Aggregator -> FileWriter.

The program does not guarantee that reported trades are unique across starts. If this is
desirable, the program can be extended to read the output file and builds a lookup table to
detect duplicate trades across starts.
