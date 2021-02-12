# Contributing to tindicators

You'd like to help contribute to this project? Excellent! 
Here is how you can do so. Read carefully.

## 1. Library design

Technical indicator is a function which is calculated for every point
of a time series considering the point's preceding history. Bar is a point of
the time series. Time series are represented as a set of arrays of
doubles, each array representing a bar component. The following components 
are allowed: `open`, `high`, `low`, `close`, `volume`, `series` (the latter can be used when it is the only component). Each indicator has zero or more options (such as a lookback period or some coefficient). Each indicator produces one or more output arrays.

Each indicator has at least one implementation, named `ti_xxx` 
(assuming xxx as the indicator name), which is written to be performant.
It can also have a reference implementation `ti_xxx_ref`, written in
a trivial way, which is used to match against in tests.

Some indicators do also have a streaming (stateful) implementation. 
The state is stored in `struct ti_stream_xxx : ti_stream { ... }`. It is 
allocated/constructed in `ti_xxx_stream_new` and destructed/deallocated 
in `ti_xxx_stream_free`. Call to `ti_xxx_stream_run` computes the 
indicator for new data.

Helper function `ti_xxx_start` tells how much input the indicator will consume without producing output.  

The indicator implementations live in the `indicators/` dir.

The index of indicators available in the library is stored in
`indicators.yaml`. Each entry describes the following properties of
an indicator:
- short name; elaborated name;
- type (unused, kept for historical reasons);
- inputs, options, outputs (their names);
- if there are reference or streaming implementations provided;
- original source of the indicator definition.

Based on this index, `codegen.py` generates `indicators.h` (the header
file of the whole library) and `indicators_index.c` (supplementary 
table of indicators; it replicates indicators.yaml and adds
support for querying indicators by name at runtime). Whenever
`codegen.py` finds an entry in indicators.yaml which does
not have a corresponding implementation, it generates the boilerplate
and invites the programmer to write one.


### Test suites
- benchmark2:
    - checks different implementations for producing equal results,
    - benchmarks them.
- fuzzer: feeds in wild combinations of options, tries to crash the implementation;
- smoke: matches outputs against precomputed values. 

Debug configuration on Linux enables the following sanitizers: `-fsanitize=undefined`, `-fsanitize=address`, `-fsanitize=leak`.

## 2. Implementing an indicator

Dependencies: clang with C++17 support, cmake, python3.

### 1. Find a source

Find a reliable (preferably, original) source like a book or a publication at traders.com.

### 2. Generate a template

Add an entry to `indicators.yaml` and run `codegen.py`.

When naming inputs, options, and outputs, follow these conventions:
- input names must be a subset of `open`, `high`, `low`, `close`, `volume`, `series` (series is allowed only when it's the only input);
- option names should be taken from the original definition, but the lookback period should be named 'period', not length or something;
- output names should start with the short name of the indicator;
- neither of the names can start with a digit nor contain spaces.

### 3. Implement the indicator

Checkout a new branch. Write an implementation in `indicators/xxx.cc`.
You are encouraged to look at the recently added indicators for 
some idiomatic constructs.

When implementing `ti_xxx_ref`, please keep as close to the definition
as possible. When implementing the other two variants, make them as
much performant as possible. In particular, choose the optimal data
structures and compute everything in one pass. 

Some util macros and data structures you may find useful: 
[ringbuf.hh](./utils/ringbuf.hh), [log.h](./utils/log.h).

Don't forget to add a custom option setter in `benchmark2.c` if needed.

### 4. Test your implementation

If the author provides an example computation, add it to 
`smoke-testcases/extra.txt`. It will be matched against in the smoke suite. 

Make sure your implementation passes tests in debug mode:
```bash
# Build:
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j

# Test:
LD_PRELOAD=libasan.so.5 ./benchmark2 xxx
ctest --output-on-failure -R 'fuzzer|smoke'
```

### 5. Submit a PR

**Squash your commits into a single one.**

Your PR must contain:
- the source from which you've taken the definition,
- a screenshot of the definition,
- clarifying comments, optionally.

You may be asked to sign a CLA.

### 6. Wait for CI checks

Wait for the tests to finish. Btw, our testing server is configured 
in such a way that when testing a PR, it runs only the relevant tests
from the test suites, based on the branch name: if the first word
in the dash-separated third slash-separated part of the branch name
is the name of an indicator, it is considered to be the *current* 
indicator and the only one to be tested. For instance:
- `feature/DEV-230/sma` will run only tests for the sma indicator,
- `feature/DEV-230/sma-fix` will do so as well,
- `feature/DEV-230/fix-sma` won't do (until we've got an indicator named 'fix').

Name your branches wisely.

### Bonus: a minimal checklist for PR review

- [x] Tests are green
- [x] Reference implementation matches the original definition
- [x] Vectorized implementation goes in one pass, performs no unnecessary allocations
- [x] Optimal data structures and algorithms are used in both vectorized and streaming implementations
- [x] Naming conventions are respected in `indicators.yaml`
- [x] Commit history is cleaned up
- [x] CLA is signed
