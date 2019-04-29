Forked from [https://github.com/TulipCharts/tulipindicators](https://github.com/TulipCharts/tulipindicators)

-------------------------------

## Bindings
There are some nice bindings available, for use in [Python](https://github.com/hcmc-project/tulipindicators-python) and [Lean](https://github.com/hcmc-project/tulipindicators-net).

## Testing

The testing process consists of three stages:
1. **`benchmark2`**: a series of 4000 bars is randomly generated, then the behavior of different implementations (plain, `ref`, `stream`) is matched against each other, and also benchmarked.
2. **`fuzzer`**: trying to find options that would trigger a segfault, memleak, of something alike.
3. **`smoke`**: we match the behavior of the indicator against precomputed values.

With debug build on Linux, these are run under sanitizers, namely `-fsanitize=undefined -fsanitize=address -fsanitize=leak`.

## Architecture

1. **indicators.tcl**: this is the heart of the project, it's the script that generates
    + the boilerplate under `indicators/` for further indicator implementation
    + `indicators.h`
    + `indicators_index.c`
2. **indicators/xxx.c**: this is the file implementing *xxx*, namely:
    + `int xxx(int size, double **inputs, double *options, double **outputs)`
    + `int xxx_start(double *options)`  
    *and optionally,*
    + `int xxx_stream_new(double *options, ti_stream **stream)`
    + `int xxx_stream_run(ti_stream *stream, int size, double **inputs, double **outputs)`
    + `void xxx_stream_free(ti_stream *stream)`   
    *and*
    + `int xxx_ref(int size, double **inputs, double *options, double **outputs)`  

-------------------------------

# Tulip Indicators

## Introduction

Tulip Indicators is a library of technical analysis functions written in ANSI C.

Lots of information is available on the website:
[https://tulipindicators.org](https://tulipindicators.org)

High quality bindings are available for [Node.js](https://github.com/TulipCharts/tulipnode),
[Python](https://github.com/cirla/tulipy), and [.NET](https://github.com/TulipCharts/tinet).

## Features

 - **C99 with no dependencies**.
 - Uses fast algorithms.
 - Easy to use programming interface.
 - Release under LGPL license.


## Building

Building is easy. You only need a decent C compiler. Tulip Indicators has no
other dependencies.

Just download the code and run `make`.

```
git clone https://github.com/TulipCharts/tulipindicators
cd tulipindicators
make
```

You should get a static library, `libindicators.a`. You'll need that library
and the header file `indicators.h` to use Tulip Indicators in your code.

## Usage

For usage information, please see:
[https://tulipindicators.org/usage](https://tulipindicators.org/usage)


## Indicator Listing
```
104 total indicators

Overlay
   avgprice            Average Price
   bbands              Bollinger Bands
   dema                Double Exponential Moving Average
   ema                 Exponential Moving Average
   hma                 Hull Moving Average
   kama                Kaufman Adaptive Moving Average
   linreg              Linear Regression
   medprice            Median Price
   psar                Parabolic SAR
   sma                 Simple Moving Average
   tema                Triple Exponential Moving Average
   trima               Triangular Moving Average
   tsf                 Time Series Forecast
   typprice            Typical Price
   vidya               Variable Index Dynamic Average
   vwma                Volume Weighted Moving Average
   wcprice             Weighted Close Price
   wilders             Wilders Smoothing
   wma                 Weighted Moving Average
   zlema               Zero-Lag Exponential Moving Average

Indicator
   ad                  Accumulation/Distribution Line
   adosc               Accumulation/Distribution Oscillator
   adx                 Average Directional Movement Index
   adxr                Average Directional Movement Rating
   ao                  Awesome Oscillator
   apo                 Absolute Price Oscillator
   aroon               Aroon
   aroonosc            Aroon Oscillator
   atr                 Average True Range
   bop                 Balance of Power
   cci                 Commodity Channel Index
   cmo                 Chande Momentum Oscillator
   cvi                 Chaikins Volatility
   di                  Directional Indicator
   dm                  Directional Movement
   dpo                 Detrended Price Oscillator
   dx                  Directional Movement Index
   emv                 Ease of Movement
   fisher              Fisher Transform
   fosc                Forecast Oscillator
   kvo                 Klinger Volume Oscillator
   linregintercept     Linear Regression Intercept
   linregslope         Linear Regression Slope
   macd                Moving Average Convergence/Divergence
   marketfi            Market Facilitation Index
   mass                Mass Index
   mfi                 Money Flow Index
   mom                 Momentum
   msw                 Mesa Sine Wave
   natr                Normalized Average True Range
   nvi                 Negative Volume Index
   obv                 On Balance Volume
   ppo                 Percentage Price Oscillator
   pvi                 Positive Volume Index
   qstick              Qstick
   roc                 Rate of Change
   rocr                Rate of Change Ratio
   rsi                 Relative Strength Index
   stoch               Stochastic Oscillator
   stochrsi            Stochastic RSI
   tr                  True Range
   trix                Trix
   ultosc              Ultimate Oscillator
   vhf                 Vertical Horizontal Filter
   volatility          Annualized Historical Volatility
   vosc                Volume Oscillator
   wad                 Williams Accumulation/Distribution
   willr               Williams %R

Math
   crossany            Crossany
   crossover           Crossover
   decay               Linear Decay
   edecay              Exponential Decay
   lag                 Lag
   max                 Maximum In Period
   md                  Mean Deviation Over Period
   min                 Minimum In Period
   stddev              Standard Deviation Over Period
   stderr              Standard Error Over Period
   sum                 Sum Over Period
   var                 Variance Over Period

Simple
   abs                 Vector Absolute Value
   acos                Vector Arccosine
   add                 Vector Addition
   asin                Vector Arcsine
   atan                Vector Arctangent
   ceil                Vector Ceiling
   cos                 Vector Cosine
   cosh                Vector Hyperbolic Cosine
   div                 Vector Division
   exp                 Vector Exponential
   floor               Vector Floor
   ln                  Vector Natural Log
   log10               Vector Base-10 Log
   mul                 Vector Multiplication
   round               Vector Round
   sin                 Vector Sine
   sinh                Vector Hyperbolic Sine
   sqrt                Vector Square Root
   sub                 Vector Subtraction
   tan                 Vector Tangent
   tanh                Vector Hyperbolic Tangent
   todeg               Vector Degree Conversion
   torad               Vector Radian Conversion
   trunc               Vector Truncate
```


## Special Thanks

The stochrsi indicator was sponsored by: [Gunthy](https://gunthy.org).
