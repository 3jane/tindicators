# Contributing to Tulip Indicators

## 1. Find a source

Find a reliable source like a book or a publication at [traders.com](https://traders.com).

The google dork `site:traders.com` might be of use. [Library Genesis](https://libgen.is) might also help.

When looking for info at traders.com, please keep in mind that you need the original article, not the Traders' Tips or whatever. Articles are generally paywalled, our company pays for it.

## 2. Create a branch

Create a branch according to our [conventions](https://github.com/hcmc-project/docs/blob/master/git.md), e.g.:

```
git checkout -b feature/DEV-230/sma 0.9-dev
```

## 3. Short note on the library design

**Definition.** Technical indicator is a function which is calculated for every point of the [time series](https://en.wikipedia.org/wiki/Time_series) considering its preceding history.

**Definition.** Bar is a point of the time series.

Tulip Indicators operates on time series represented as a set of arrays of floating point numbers, `double[]`. Each array corresponds to a bar component, one of the following:
- open
- high
- low
- close
- volume

And a special component `real`, which is used when it's the only component of the bar.

Indicators do also have parameters, or *options*, such as a lookback period or some coefficient. Indicators might also have multiple *outputs*.

In the library, an indicator is represented as a function taking the following arguments:
- size: the size of the time series
- inputs: an array of pointers to the time series components
- options: an array of options
- outputs: an array of pointers to the output arrays

Every indicator is implemented in two variants: the "workhorse", most performant possible, and the *reference*, most trivial and close-to-definition possible. They are then tested for producing equal results. They are named `ti_xxx` and `ti_xxx_ref`, respectively, *xxx* being the short name of the indicator.

Every indicator does also have a stateful, or *streaming*, implementation. The caller is able to feed in data and receive results on the go. Its state is stored in a `ti_stream` structure. The first two members of the structure are required to be the following, in order:
- int index: the index of the indicator to the runtime table of indicators (discussed later)
- int progress: how many bars have been processed so far

The rest of the layout is unspecified and is up to the implementator to decide. A convenient template is automatically generated, however. The state object is created inside the `ti_xxx_stream_new` function, which takes the options array, and destroyed inside `ti_xxx_stream_free`. The stateful indicator implementation is called `ti_xxx_stream_run`, and takes the state object and all the same arguments except options. The quirk of the streaming implementation is that you only have access to `size` bars, usually `size=1`, so you need to save the relevant piece of history manually.

(Actually, the older indicators are not equipped with the reference and streaming implementations, but our goal is to equip them all.)

Additionally, there is the function `ti_xxx_start` associated with each indicator, which tells the caller how many bars the indicator will consume on the start without producing output.

All the implementations of the indicator xxx are put in `indicators/xxx.c` or `indicators/xxx.cc`, depending on the language, C or C++, respectively.

The ultimate list of the indicators present in the library is contained in `indicators.yaml`. Each entry describes the following properties of the indicator:
- short name, elaborated name
- type (meaningless classification, put here whatever you want)
- inputs, options, outputs: their names
- extra features: if there are *ref* or *stream* implementations provided

Based on this list, the script called `codegen.py` generates the header file of all of the library, named `indicators.h`, and the runtime table of indicators, named `indicators_index.c`. This table fully replicates the data from `indicators.yaml` and adds the ability to query&run the indicator by name at runtime, a feature which is heavily used in e.g. the Python binding.

Another highly useful thing `codegen.py` does is the template generation for future indicator implementation: once it processes an entry and is unable to find neither of C and C++ corresponding implementation files, it generates a template. By default, it does so for a C++ implementation, but you may opt-in C with the `--old` switch.

Implementing an indicator, it's important to handle basic errors, those which lead to a segfault or incorrect behavior. There are three constants defined: `TI_OK`, `TI_INVALID_OPTION`, `TI_OUT_OF_MEMORY`. Every function except of `ti_xxx_start` must return one of the three.

As this library is the heart of our technology, it's crucial to perform proper testing. There are the following test stages currently employed:
- benchmark2: verifies that all of the implementations produce the same results on a given series, and benchmarks the code
- fuzzer: verifies the robustness by feeding in various options and trying to crash the indicator
- smoke: verifies that the indicator produces the same result as the precomuted values in `tests/`

Built in debug mode on Linux, the project enables the following sanitizers: `-fsanitize=undefined`, `-fsanitize=address`, `-fsanitize=leak`. Our CI/CD server also runs the tests under valgrind.

## 4. Generate a template

Add an entry to `indicators.yaml` and run `codegen.py`.

When naming inputs, options, and outputs, please stick to the following conventions:
- input names should be ones of those listed in the [Short note](#3-short-note-on-the-library-design)
- option names should generally be taken from the definition, with one exception: the lookback period is always named 'period', not 'length' or whatever
- output names should start with the short name of the indicator

Also, neither of them can start with a digit nor contain spaces.

## 5. Implement the indicator

Go to `indicators/xxx.cc` and implement the indicator. You are encouraged to look at the recently added indicators for some idiomatic constructs.

When implementing `ti_xxx_ref`, please keep as close to the definition as possible. When implementing the other two variants, make them as much performant as possible. In particular, choose the optimal data structures and compute everything in one pass. 

Please keep in mind you are expected to implement all the three variants. Even in case `ti_xxx` matches the ref version, duplicate it, as the ref version is built without any optimizations, which is needed to verify our compiler haven't broken anything while optimizing the rest.

There are several macros and data structures you might find useful:
- [ringbuf.hh](./utils/ringbuf.hh)
- [log.h](./utils/log.h)
- [localbuffer.h](./utils/localbuffer.h)
- [buffer.h](./utils/buffer.h)

## 6. Add a precomputed test

Consider adding a precomputed testcase to `tests/extra.txt` if the author provides one. It's often not the case, and we currently don't do anything about it, but ultimately, we will need to resolve this in one way or another.

## 7. Test your implementation

Make sure your implementation passes tests in debug mode. Unfortunately, our CI/CD server doesn't support libasan, so it's up to you to build and test this configuration. It will produce a readable error in case the sanitizers detect something.

Build:

```
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . -j
```

Test:

```
LD_PRELOAD=libasan.so.5 ./benchmark2 xxx
ctest --output-on-failure -R 'fuzzer|smoke'
```

## 8. Create a Pull Request

First, squash your commits into one. Push it to the remote.

Your PR must contain:
- the source from which you've taken the definition
- a screenshot of the definition
- additional comments, optionally

See older [PRs](https://github.com/hcmc-project/tulipindicators-private/pull/7) for an example.

## 9. Wait for tests

Wait for the tests to finish. By the way, our CI/CD server is configured in such a way that when testing a PR, it runs only the relevant tests from the test suite, based on the branch name: if the first word in the dash-separated third slash-separated part of the branch name is the name of an indicator, it is considered the current indicator and is the only one being tested, for example:
- `feature/DEV-230/sma` will run only tests for the sma indicator
- `feature/DEV-230/sma-fix` will do so as well
- `feature/DEV-230/fix-sma` won't do (until we've got an indicator named 'fix')

Therefore, you're encouraged to name the branches properly.

## 10. Wait for a review

Update the [sheet](https://docs.google.com/spreadsheets/d/1WhdTc_AN-_KF_tgcG8B31Tgy6z-pR9rOv1Nr3dxLn5g/edit#gid=444645194) and ping **@ilya.p**.

## 11. Checklist for a review

- [x] Tests are green
- [x] Reference implementation complies with the spec
- [x] Vectorized implementation goes in one pass and performs no unnecessary allocations
- [x] Optimal data structures and algorithms are used in both vectorized and streaming implementations
- [x] Naming conventions are respected in `indicators.yaml`
- [x] Commit history is cleaned up