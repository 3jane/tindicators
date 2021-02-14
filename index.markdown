---
layout: page
title: Documentation
---

## Quick start
```bash
pip install tindicators
```
```python
from tindicators import ti
```

Example usage:
```python
>>> import numpy as np
>>> ti.sma(np.arange(10), 3)
array([nan, nan,  1.,  2.,  3.,  4.,  5.,  6.,  7.,  8.])
```

## List of available indicators
There are currently **161 indicators** available in [tindicators](https://github.com/3jane/tindicators) v0.9.12.

#### Double Exponential Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [dema.cc](https://github.com/3jane/tindicators/indicators/dema.cc) <br/>
*Signature:*

```python
ti.dema(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    dema = np.ndarray
)
```
	
#### Exponential Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [ema.cc](https://github.com/3jane/tindicators/indicators/ema.cc) <br/>
*Signature:*

```python
ti.ema(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    ema = np.ndarray
)
```
	
#### Hampel Filter on Simple Moving Average
*Source:* Pearson, Neuvo, Astola, Gabbouj, Generalized Hampel Filters <br/>
*Implementation:* [hfsma.cc](https://github.com/3jane/tindicators/indicators/hfsma.cc) <br/>
*Signature:*

```python
ti.hfsma(
    series: np.ndarray,
    sma_period,
    k,
    threshold
) -> NamedTuple(...,
    hfsma = np.ndarray
)
```
	
#### Hull Moving Average
*Source:* - <br/>
*Implementation:* [hma.cc](https://github.com/3jane/tindicators/indicators/hma.cc) <br/>
*Signature:*

```python
ti.hma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    hma = np.ndarray
)
```
	
#### Hampel Filter on Exponential Moving Average
*Source:* Pearson, Neuvo, Astola, Gabbouj, Generalized Hampel Filters <br/>
*Implementation:* [hfema.cc](https://github.com/3jane/tindicators/indicators/hfema.cc) <br/>
*Signature:*

```python
ti.hfema(
    series: np.ndarray,
    ema_period,
    k,
    threshold
) -> NamedTuple(...,
    hfema = np.ndarray
)
```
	
#### Simple Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [sma.cc](https://github.com/3jane/tindicators/indicators/sma.cc) <br/>
*Signature:*

```python
ti.sma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    sma = np.ndarray
)
```
	
#### Laguerre Filter
*Source:* Ehlers, Time Warp - Without Space Travel <br/>
*Implementation:* [lf.cc](https://github.com/3jane/tindicators/indicators/lf.cc) <br/>
*Signature:*

```python
ti.lf(
    series: np.ndarray,
    gamma
) -> NamedTuple(...,
    lf = np.ndarray
)
```
	
#### Recursive Median Filter
*Source:* Stocks & Commodities V. 36:03 (8–11): Recursive Median Filters by John F. Ehlers <br/>
*Implementation:* [rmf.cc](https://github.com/3jane/tindicators/indicators/rmf.cc) <br/>
*Signature:*

```python
ti.rmf(
    series: np.ndarray,
    critical_period,
    median_period
) -> NamedTuple(...,
    rmf = np.ndarray
)
```
	
#### MESA Adaptive Moving Average
*Source:* Ehlers. Rocket Science for Traders, pp. 182-183 <br/>
*Implementation:* [mama.cc](https://github.com/3jane/tindicators/indicators/mama.cc) <br/>
*Signature:*

```python
ti.mama(
    series: np.ndarray,
    fastlimit,
    slowlimit
) -> NamedTuple(...,
    mama = np.ndarray,
    fama = np.ndarray
)
```
	
#### Regularized Exponential Moving Average
*Source:* Satchwell. Regularization. Stocks & Commodities V. 21:7 (38-41) <br/>
*Implementation:* [rema.cc](https://github.com/3jane/tindicators/indicators/rema.cc) <br/>
*Signature:*

```python
ti.rema(
    series: np.ndarray,
    period,
    lambda
) -> NamedTuple(...,
    rema = np.ndarray
)
```
	
#### Triple Exponential Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [tema.cc](https://github.com/3jane/tindicators/indicators/tema.cc) <br/>
*Signature:*

```python
ti.tema(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    tema = np.ndarray
)
```
	
#### Triangular Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [trima.cc](https://github.com/3jane/tindicators/indicators/trima.cc) <br/>
*Signature:*

```python
ti.trima(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    trima = np.ndarray
)
```
	
#### Variable Index Dynamic Average
*Source:* - <br/>
*Implementation:* [vidya.cc](https://github.com/3jane/tindicators/indicators/vidya.cc) <br/>
*Signature:*

```python
ti.vidya(
    series: np.ndarray,
    short period,
    long period,
    alpha
) -> NamedTuple(...,
    vidya = np.ndarray
)
```
	
#### Volume Weighted Moving Average
*Source:* - <br/>
*Implementation:* [vwma.cc](https://github.com/3jane/tindicators/indicators/vwma.cc) <br/>
*Signature:*

```python
ti.vwma(
    close: np.ndarray,
    volume: np.ndarray,
    period
) -> NamedTuple(...,
    vwma = np.ndarray
)
```
	
#### Volume Weighted Average Price
*Source:* - <br/>
*Implementation:* [vwap.cc](https://github.com/3jane/tindicators/indicators/vwap.cc) <br/>
*Signature:*

```python
ti.vwap(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray,
    period
) -> NamedTuple(...,
    vwap = np.ndarray
)
```
	
#### Wilders Smoothing
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [wilders.cc](https://github.com/3jane/tindicators/indicators/wilders.cc) <br/>
*Signature:*

```python
ti.wilders(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    wilders = np.ndarray
)
```
	
#### Weighted Moving Average
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [wma.cc](https://github.com/3jane/tindicators/indicators/wma.cc) <br/>
*Signature:*

```python
ti.wma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    wma = np.ndarray
)
```
	
#### Zero-Lag Exponential Moving Average
*Source:* - <br/>
*Implementation:* [zlema.cc](https://github.com/3jane/tindicators/indicators/zlema.cc) <br/>
*Signature:*

```python
ti.zlema(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    zlema = np.ndarray
)
```
	
#### Recursive Moving Trend Average
*Source:* Meyers. The Japanese Yen, Recursed <br/>
*Implementation:* [rmta.cc](https://github.com/3jane/tindicators/indicators/rmta.cc) <br/>
*Signature:*

```python
ti.rmta(
    series: np.ndarray,
    period,
    beta
) -> NamedTuple(...,
    rmta = np.ndarray
)
```
	
#### Gaussian Filter - 1 Pole
*Source:* Gaussian and Other Low Lag Filters - John Ehlers <br/>
*Implementation:* [gf1.cc](https://github.com/3jane/tindicators/indicators/gf1.cc) <br/>
*Signature:*

```python
ti.gf1(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    gf1 = np.ndarray
)
```
	
#### Gaussian Filter - 2 Poles
*Source:* Gaussian and Other Low Lag Filters - John Ehlers <br/>
*Implementation:* [gf2.cc](https://github.com/3jane/tindicators/indicators/gf2.cc) <br/>
*Signature:*

```python
ti.gf2(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    gf2 = np.ndarray
)
```
	
#### Gaussian Filter - 3 Poles
*Source:* Gaussian and Other Low Lag Filters - John Ehlers <br/>
*Implementation:* [gf3.cc](https://github.com/3jane/tindicators/indicators/gf3.cc) <br/>
*Signature:*

```python
ti.gf3(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    gf3 = np.ndarray
)
```
	
#### Gaussian Filter - 4 Poles
*Source:* Gaussian and Other Low Lag Filters - John Ehlers <br/>
*Implementation:* [gf4.cc](https://github.com/3jane/tindicators/indicators/gf4.cc) <br/>
*Signature:*

```python
ti.gf4(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    gf4 = np.ndarray
)
```
	
#### Exponential Hull Moving Average
*Source:* Raudys, A., Lenčiauskas, V., & Malčius, E. (2013). Moving Averages for Financial Data Smoothing. Information and Software Technologies, 34–45. doi:10.1007/978-3-642-41947-8_4  <br/>
*Implementation:* [ehma.cc](https://github.com/3jane/tindicators/indicators/ehma.cc) <br/>
*Signature:*

```python
ti.ehma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    ehma = np.ndarray
)
```
	
#### Elastic Volume Weighted Moving Average
*Source:* The Distribution of Share Prices and Elastic Time and Volume Weighted Moving Averages, Christian P. Fries, preprint <br/>
*Implementation:* [evwma.cc](https://github.com/3jane/tindicators/indicators/evwma.cc) <br/>
*Signature:*

```python
ti.evwma(
    close: np.ndarray,
    volume: np.ndarray,
    period,
    gamma
) -> NamedTuple(...,
    evwma = np.ndarray
)
```
	
#### Leo Moving Average
*Source:* LEO MOVING AVERAGE + SUPPORT/RESISTANCE <br/>
*Implementation:* [lma.cc](https://github.com/3jane/tindicators/indicators/lma.cc) <br/>
*Signature:*

```python
ti.lma(
    close: np.ndarray,
    period
) -> NamedTuple(...,
    lma = np.ndarray
)
```
	
#### Power Weighted Moving Average
*Source:* - <br/>
*Implementation:* [pwma.cc](https://github.com/3jane/tindicators/indicators/pwma.cc) <br/>
*Signature:*

```python
ti.pwma(
    series: np.ndarray,
    period,
    power
) -> NamedTuple(...,
    pwma = np.ndarray
)
```
	
#### Sine Weighted Moving Average
*Source:* Raudys, A., Lenčiauskas, V., & Malčius, E. (2013). Moving Averages for Financial Data Smoothing. Information and Software Technologies, 34–45. doi:10.1007/978-3-642-41947-8_4  <br/>
*Implementation:* [swma.cc](https://github.com/3jane/tindicators/indicators/swma.cc) <br/>
*Signature:*

```python
ti.swma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    swma = np.ndarray
)
```
	
#### Double Weighted Moving Average
*Source:* Stocks & Commodities V. 12:1 (11-19): Smoothing Data With Faster Moving Averages by Patrick G. Mulloy <br/>
*Implementation:* [dwma.cc](https://github.com/3jane/tindicators/indicators/dwma.cc) <br/>
*Signature:*

```python
ti.dwma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    dwma = np.ndarray
)
```
	
#### Sharp Modified Moving Average
*Source:* V.18:1 (56-60): More Responsive Moving Averages by Joe Sharp,Ph.D. <br/>
*Implementation:* [shmma.cc](https://github.com/3jane/tindicators/indicators/shmma.cc) <br/>
*Signature:*

```python
ti.shmma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    shmma = np.ndarray
)
```
	
#### Ahrens Moving Average
*Source:* Stocks & Commodities V. 31:10 (26-30): Build A Better Moving Average by Richard D. Ahrens <br/>
*Implementation:* [ahma.cc](https://github.com/3jane/tindicators/indicators/ahma.cc) <br/>
*Signature:*

```python
ti.ahma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    ahma = np.ndarray
)
```
	
#### T3 Moving Average
*Source:* V.16:1 (33-37): Smoothing Techniques For More Accurate Signals by Tim Tillson <br/>
*Implementation:* [t3.cc](https://github.com/3jane/tindicators/indicators/t3.cc) <br/>
*Signature:*

```python
ti.t3(
    series: np.ndarray,
    period,
    v
) -> NamedTuple(...,
    t3 = np.ndarray
)
```
	
#### Middle-High-Low Moving Average
*Source:* Stocks & Commodities V. 34:08 (26–29): The Middle-High-Low Moving Average by Vitali Apirine <br/>
*Implementation:* [mhlma.cc](https://github.com/3jane/tindicators/indicators/mhlma.cc) <br/>
*Signature:*

```python
ti.mhlma(
    series: np.ndarray,
    period,
    ma_period
) -> NamedTuple(...,
    mhlsma = np.ndarray,
    mhlema = np.ndarray
)
```
	
#### Choppy Market Indicator
*Source:* Trading Techniques. Measuring market choppiness with chaos <br/>
*Implementation:* [cmi.cc](https://github.com/3jane/tindicators/indicators/cmi.cc) <br/>
*Signature:*

```python
ti.cmi(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    cmi = np.ndarray
)
```
	
#### Inverse Distance Weighted Moving Average
*Source:* Shepard, D. (1968). A two-dimensional interpolation function for irregularly-spaced data. Proceedings of the 1968 23rd ACM National Conference on -. doi:10.1145/800186.810616  <br/>
*Implementation:* [idwma.cc](https://github.com/3jane/tindicators/indicators/idwma.cc) <br/>
*Signature:*

```python
ti.idwma(
    series: np.ndarray,
    period,
    exponent
) -> NamedTuple(...,
    idwma = np.ndarray
)
```
	
#### Exponential Moving Standard Deviation
*Source:* Mathworks. Moving Standard Deviation <br/>
*Implementation:* [emsd.cc](https://github.com/3jane/tindicators/indicators/emsd.cc) <br/>
*Signature:*

```python
ti.emsd(
    series: np.ndarray,
    period,
    ma_period
) -> NamedTuple(...,
    emsd = np.ndarray
)
```
	
#### Henderson asymmetric filter
*Source:* Australian Bureau of Statistics. Time Series Analysis: The Process of Seasonal Adjustment <br/>
*Implementation:* [hwma.cc](https://github.com/3jane/tindicators/indicators/hwma.cc) <br/>
*Signature:*

```python
ti.hwma(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    hwma = np.ndarray
)
```
	
#### Homodyne Discriminator
*Source:* Ehlers. Rocket Science for Traders, pp. 68-69 <br/>
*Implementation:* [hd.cc](https://github.com/3jane/tindicators/indicators/hd.cc) <br/>
*Signature:*

```python
ti.hd(
    series: np.ndarray
) -> NamedTuple(...,
    hd = np.ndarray
)
```
	
#### McGinley Dynamic
*Source:* Stocks & Commodities V. 28:3 (30-37): The McGinley Dynamic by Brian Twomey <br/>
*Implementation:* [mgdyn.cc](https://github.com/3jane/tindicators/indicators/mgdyn.cc) <br/>
*Signature:*

```python
ti.mgdyn(
    series: np.ndarray,
    N
) -> NamedTuple(...,
    mgdyn = np.ndarray
)
```
	
#### Linear Regression
*Source:* - <br/>
*Implementation:* [linreg.cc](https://github.com/3jane/tindicators/indicators/linreg.cc) <br/>
*Signature:*

```python
ti.linreg(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    linreg = np.ndarray
)
```
	
#### Linear Regression Intercept
*Source:* - <br/>
*Implementation:* [linregintercept.cc](https://github.com/3jane/tindicators/indicators/linregintercept.cc) <br/>
*Signature:*

```python
ti.linregintercept(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    linregintercept = np.ndarray
)
```
	
#### Linear Regression Slope
*Source:* - <br/>
*Implementation:* [linregslope.cc](https://github.com/3jane/tindicators/indicators/linregslope.cc) <br/>
*Signature:*

```python
ti.linregslope(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    linregslope = np.ndarray
)
```
	
#### Time Series Forecast
*Source:* - <br/>
*Implementation:* [tsf.cc](https://github.com/3jane/tindicators/indicators/tsf.cc) <br/>
*Signature:*

```python
ti.tsf(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    tsf = np.ndarray
)
```
	
#### Forecast Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [fosc.cc](https://github.com/3jane/tindicators/indicators/fosc.cc) <br/>
*Signature:*

```python
ti.fosc(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    fosc = np.ndarray
)
```
	
#### Bollinger Bands
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [bbands.cc](https://github.com/3jane/tindicators/indicators/bbands.cc) <br/>
*Signature:*

```python
ti.bbands(
    series: np.ndarray,
    period,
    stddev
) -> NamedTuple(...,
    bbands_lower = np.ndarray,
    bbands_middle = np.ndarray,
    bbands_upper = np.ndarray
)
```
	
#### Butterworth Filter - 2 Poles
*Source:* Ehlers. POLES, ZEROS, and HIGHER ORDER FILTERS <br/>
*Implementation:* [bf2.cc](https://github.com/3jane/tindicators/indicators/bf2.cc) <br/>
*Signature:*

```python
ti.bf2(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    bf2 = np.ndarray
)
```
	
#### Butterworth Filter - 3 Poles
*Source:* Ehlers. POLES, ZEROS, and HIGHER ORDER FILTERS <br/>
*Implementation:* [bf3.cc](https://github.com/3jane/tindicators/indicators/bf3.cc) <br/>
*Signature:*

```python
ti.bf3(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    bf3 = np.ndarray
)
```
	
#### Keltner Channel
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 337 <br/>
*Implementation:* [kc.cc](https://github.com/3jane/tindicators/indicators/kc.cc) <br/>
*Signature:*

```python
ti.kc(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period,
    multiple
) -> NamedTuple(...,
    kc_lower = np.ndarray,
    kc_middle = np.ndarray,
    kc_upper = np.ndarray
)
```
	
#### Kaufman Adaptive Moving Average
*Source:* - <br/>
*Implementation:* [kama.cc](https://github.com/3jane/tindicators/indicators/kama.cc) <br/>
*Signature:*

```python
ti.kama(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    kama = np.ndarray
)
```
	
#### Parabolic SAR
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [psar.cc](https://github.com/3jane/tindicators/indicators/psar.cc) <br/>
*Signature:*

```python
ti.psar(
    high: np.ndarray,
    low: np.ndarray,
    acceleration factor step,
    acceleration factor maximum
) -> NamedTuple(...,
    psar = np.ndarray
)
```
	
#### Price Channel
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 534 <br/>
*Implementation:* [pc.cc](https://github.com/3jane/tindicators/indicators/pc.cc) <br/>
*Signature:*

```python
ti.pc(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    pc_low = np.ndarray,
    pc_high = np.ndarray
)
```
	
#### Projection Bands
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 545 <br/>
*Implementation:* [pbands.cc](https://github.com/3jane/tindicators/indicators/pbands.cc) <br/>
*Signature:*

```python
ti.pbands(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    pbands_lower = np.ndarray,
    pbands_upper = np.ndarray
)
```
	
#### Ehlers Distance Coefficient Filter
*Source:* Ehlers. Rocket Science for Traders, p.193 <br/>
*Implementation:* [edcf.cc](https://github.com/3jane/tindicators/indicators/edcf.cc) <br/>
*Signature:*

```python
ti.edcf(
    series: np.ndarray,
    length
) -> NamedTuple(...,
    edcf = np.ndarray
)
```
	
#### Fractal Adaptive Moving Average
*Source:* Ehlers, FRAMA – Fractal Adaptive Moving Average <br/>
*Implementation:* [frama.cc](https://github.com/3jane/tindicators/indicators/frama.cc) <br/>
*Signature:*

```python
ti.frama(
    high: np.ndarray,
    low: np.ndarray,
    period,
    average_period
) -> NamedTuple(...,
    frama = np.ndarray
)
```
	
#### The Roofing Filter
*Source:* Stocks & Commodities V. 32:1 (16-25): Predictive And Successful Indicators by John F. Ehlers, PhD <br/>
*Implementation:* [roof.cc](https://github.com/3jane/tindicators/indicators/roof.cc) <br/>
*Signature:*

```python
ti.roof(
    series: np.ndarray
) -> NamedTuple(...,
    roof = np.ndarray
)
```
	
#### SuperSmoother Filter
*Source:* Stocks & Commodities V. 32:1 (16-25): Predictive And Successful Indicators by John F. Ehlers, PhD <br/>
*Implementation:* [ssmooth.cc](https://github.com/3jane/tindicators/indicators/ssmooth.cc) <br/>
*Signature:*

```python
ti.ssmooth(
    series: np.ndarray
) -> NamedTuple(...,
    ssmooth = np.ndarray
)
```
	
#### Hampel Filter
*Source:* Pearson, Neuvo, Astola, Gabbouj, Generalized Hampel Filters <br/>
*Implementation:* [hf.cc](https://github.com/3jane/tindicators/indicators/hf.cc) <br/>
*Signature:*

```python
ti.hf(
    series: np.ndarray,
    period,
    threshold
) -> NamedTuple(...,
    hf = np.ndarray
)
```
	
#### Accumulation/Distribution Line
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [ad.cc](https://github.com/3jane/tindicators/indicators/ad.cc) <br/>
*Signature:*

```python
ti.ad(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    ad = np.ndarray
)
```
	
#### Accumulation/Distribution Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [adosc.cc](https://github.com/3jane/tindicators/indicators/adosc.cc) <br/>
*Signature:*

```python
ti.adosc(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray,
    short period,
    long period
) -> NamedTuple(...,
    adosc = np.ndarray
)
```
	
#### Acceleration Bands
*Source:* Headley. Big Trends In Trading, p. 92 <br/>
*Implementation:* [abands.cc](https://github.com/3jane/tindicators/indicators/abands.cc) <br/>
*Signature:*

```python
ti.abands(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    abands_lower = np.ndarray,
    abands_upper = np.ndarray,
    abands_middle = np.ndarray
)
```
	
#### Absolute Price Oscillator
*Source:* - <br/>
*Implementation:* [apo.cc](https://github.com/3jane/tindicators/indicators/apo.cc) <br/>
*Signature:*

```python
ti.apo(
    series: np.ndarray,
    short period,
    long period
) -> NamedTuple(...,
    apo = np.ndarray
)
```
	
#### Aroon
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [aroon.cc](https://github.com/3jane/tindicators/indicators/aroon.cc) <br/>
*Signature:*

```python
ti.aroon(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    aroon_down = np.ndarray,
    aroon_up = np.ndarray
)
```
	
#### Aroon Oscillator
*Source:* - <br/>
*Implementation:* [aroonosc.cc](https://github.com/3jane/tindicators/indicators/aroonosc.cc) <br/>
*Signature:*

```python
ti.aroonosc(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    aroonosc = np.ndarray
)
```
	
#### Awesome Oscillator
*Source:* - <br/>
*Implementation:* [ao.cc](https://github.com/3jane/tindicators/indicators/ao.cc) <br/>
*Signature:*

```python
ti.ao(
    high: np.ndarray,
    low: np.ndarray
) -> NamedTuple(...,
    ao = np.ndarray
)
```
	
#### Balance of Power
*Source:* - <br/>
*Implementation:* [bop.cc](https://github.com/3jane/tindicators/indicators/bop.cc) <br/>
*Signature:*

```python
ti.bop(
    open: np.ndarray,
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    bop = np.ndarray
)
```
	
#### Chaikin Money Flow
*Source:* Kirkpatrick, Dahlquist. Technical Analysis: The Complete Resource for Financial Market Technicians, pp. 419, 421 <br/>
*Implementation:* [cmf.cc](https://github.com/3jane/tindicators/indicators/cmf.cc) <br/>
*Signature:*

```python
ti.cmf(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray,
    period
) -> NamedTuple(...,
    cmf = np.ndarray
)
```
	
#### MESA Stochastic (by John F. Ehlers)
*Source:* Stocks & Commodities V. 32:1 (16-25): Predictive And Successful Indicators by John F. Ehlers, PhD <br/>
*Implementation:* [mesastoch.cc](https://github.com/3jane/tindicators/indicators/mesastoch.cc) <br/>
*Signature:*

```python
ti.mesastoch(
    series: np.ndarray,
    period,
    max_cycle_considered
) -> NamedTuple(...,
    mesastoch = np.ndarray
)
```
	
#### Chande Momentum Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [cmo.cc](https://github.com/3jane/tindicators/indicators/cmo.cc) <br/>
*Signature:*

```python
ti.cmo(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    cmo = np.ndarray
)
```
	
#### Commodity Channel Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [cci.cc](https://github.com/3jane/tindicators/indicators/cci.cc) <br/>
*Signature:*

```python
ti.cci(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    cci = np.ndarray
)
```
	
#### Coppock Curve
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 168 <br/>
*Implementation:* [copp.cc](https://github.com/3jane/tindicators/indicators/copp.cc) <br/>
*Signature:*

```python
ti.copp(
    series: np.ndarray,
    roc_shorter_period,
    roc_longer_period,
    wma_period
) -> NamedTuple(...,
    copp = np.ndarray
)
```
	
#### Detrended Price Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [dpo.cc](https://github.com/3jane/tindicators/indicators/dpo.cc) <br/>
*Signature:*

```python
ti.dpo(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    dpo = np.ndarray
)
```
	
#### Ease of Movement
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [emv.cc](https://github.com/3jane/tindicators/indicators/emv.cc) <br/>
*Signature:*

```python
ti.emv(
    high: np.ndarray,
    low: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    emv = np.ndarray
)
```
	
#### Fisher Transform
*Source:* - <br/>
*Implementation:* [fisher.cc](https://github.com/3jane/tindicators/indicators/fisher.cc) <br/>
*Signature:*

```python
ti.fisher(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    fisher = np.ndarray,
    fisher_signal = np.ndarray
)
```
	
#### Force Index
*Source:* Colby. The Encyclopedia of Technical Market Indicators, pp. 275, 774 <br/>
*Implementation:* [fi.cc](https://github.com/3jane/tindicators/indicators/fi.cc) <br/>
*Signature:*

```python
ti.fi(
    close: np.ndarray,
    volume: np.ndarray,
    period
) -> NamedTuple(...,
    fi = np.ndarray
)
```
	
#### Klinger Volume Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [kvo.cc](https://github.com/3jane/tindicators/indicators/kvo.cc) <br/>
*Signature:*

```python
ti.kvo(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray,
    short period,
    long period
) -> NamedTuple(...,
    kvo = np.ndarray
)
```
	
#### Know Sure Thing
*Source:* Colby. The Encyclopedia Of Technical Market Indicators, pp. 346-347 <br/>
*Implementation:* [kst.cc](https://github.com/3jane/tindicators/indicators/kst.cc) <br/>
*Signature:*

```python
ti.kst(
    series: np.ndarray,
    roc1,
    roc2,
    roc3,
    roc4,
    ma1,
    ma2,
    ma3,
    ma4
) -> NamedTuple(...,
    kst = np.ndarray,
    kst_signal = np.ndarray
)
```
	
#### Market Facilitation Index
*Source:* - <br/>
*Implementation:* [marketfi.cc](https://github.com/3jane/tindicators/indicators/marketfi.cc) <br/>
*Signature:*

```python
ti.marketfi(
    high: np.ndarray,
    low: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    marketfi = np.ndarray
)
```
	
#### Mass Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [mass.cc](https://github.com/3jane/tindicators/indicators/mass.cc) <br/>
*Signature:*

```python
ti.mass(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    mass = np.ndarray
)
```
	
#### Money Flow Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [mfi.cc](https://github.com/3jane/tindicators/indicators/mfi.cc) <br/>
*Signature:*

```python
ti.mfi(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    volume: np.ndarray,
    period
) -> NamedTuple(...,
    mfi = np.ndarray
)
```
	
#### Moving Average Convergence/Divergence
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [macd.cc](https://github.com/3jane/tindicators/indicators/macd.cc) <br/>
*Signature:*

```python
ti.macd(
    series: np.ndarray,
    short period,
    long period,
    signal period
) -> NamedTuple(...,
    macd = np.ndarray,
    macd_signal = np.ndarray,
    macd_histogram = np.ndarray
)
```
	
#### Negative Volume Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [nvi.cc](https://github.com/3jane/tindicators/indicators/nvi.cc) <br/>
*Signature:*

```python
ti.nvi(
    close: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    nvi = np.ndarray
)
```
	
#### On Balance Volume
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [obv.cc](https://github.com/3jane/tindicators/indicators/obv.cc) <br/>
*Signature:*

```python
ti.obv(
    close: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    obv = np.ndarray
)
```
	
#### Percentage Price Oscillator
*Source:* - <br/>
*Implementation:* [ppo.cc](https://github.com/3jane/tindicators/indicators/ppo.cc) <br/>
*Signature:*

```python
ti.ppo(
    series: np.ndarray,
    short period,
    long period
) -> NamedTuple(...,
    ppo = np.ndarray
)
```
	
#### Polarized Fractal Efficiency
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 520 <br/>
*Implementation:* [pfe.cc](https://github.com/3jane/tindicators/indicators/pfe.cc) <br/>
*Signature:*

```python
ti.pfe(
    series: np.ndarray,
    period,
    ema_period
) -> NamedTuple(...,
    pfe = np.ndarray
)
```
	
#### Positive Volume Index
*Source:* - <br/>
*Implementation:* [pvi.cc](https://github.com/3jane/tindicators/indicators/pvi.cc) <br/>
*Signature:*

```python
ti.pvi(
    close: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    pvi = np.ndarray
)
```
	
#### Projection Oscillator
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 545 <br/>
*Implementation:* [posc.cc](https://github.com/3jane/tindicators/indicators/posc.cc) <br/>
*Signature:*

```python
ti.posc(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period,
    ema_period
) -> NamedTuple(...,
    posc = np.ndarray
)
```
	
#### Qstick
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [qstick.cc](https://github.com/3jane/tindicators/indicators/qstick.cc) <br/>
*Signature:*

```python
ti.qstick(
    open: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    qstick = np.ndarray
)
```
	
#### Relative Strength Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [rsi.cc](https://github.com/3jane/tindicators/indicators/rsi.cc) <br/>
*Signature:*

```python
ti.rsi(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    rsi = np.ndarray
)
```
	
#### True Strength Index
*Source:* Blau. True Strength Index. Stocks & Commodities V. 9:11 (438-446).pdf <br/>
*Implementation:* [tsi.cc](https://github.com/3jane/tindicators/indicators/tsi.cc) <br/>
*Signature:*

```python
ti.tsi(
    series: np.ndarray,
    y_period,
    z_period
) -> NamedTuple(...,
    tsi = np.ndarray
)
```
	
#### Relative Momentum Index
*Source:* Relative Momentum Index: Modifying RSI by Roger Altman, Stocks and Commodities, Feb '93 <br/>
*Implementation:* [rmi.cc](https://github.com/3jane/tindicators/indicators/rmi.cc) <br/>
*Signature:*

```python
ti.rmi(
    series: np.ndarray,
    period,
    lookback_period
) -> NamedTuple(...,
    rmi = np.ndarray
)
```
	
#### Relative Volatility Index
*Source:* Colby. The Encyclopedia of Technical Market Indicators, p. 618 <br/>
*Implementation:* [rvi.cc](https://github.com/3jane/tindicators/indicators/rvi.cc) <br/>
*Signature:*

```python
ti.rvi(
    series: np.ndarray,
    ema_period,
    stddev_period
) -> NamedTuple(...,
    rvi = np.ndarray
)
```
	
#### Stochastic Momentum Index
*Source:* Blau. Stochastic Momentum. Stocks & Commodities V. 11:1 (11-18).pdf <br/>
*Implementation:* [smi.cc](https://github.com/3jane/tindicators/indicators/smi.cc) <br/>
*Signature:*

```python
ti.smi(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    q_period,
    r_period,
    s_period
) -> NamedTuple(...,
    smi = np.ndarray
)
```
	
#### Stochastic Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [stoch.cc](https://github.com/3jane/tindicators/indicators/stoch.cc) <br/>
*Signature:*

```python
ti.stoch(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    k period,
    k slowing period,
    d period
) -> NamedTuple(...,
    stoch_k = np.ndarray,
    stoch_d = np.ndarray
)
```
	
#### Stochastic RSI
*Source:* - <br/>
*Implementation:* [stochrsi.cc](https://github.com/3jane/tindicators/indicators/stochrsi.cc) <br/>
*Signature:*

```python
ti.stochrsi(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    stochrsi = np.ndarray
)
```
	
#### Trix
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [trix.cc](https://github.com/3jane/tindicators/indicators/trix.cc) <br/>
*Signature:*

```python
ti.trix(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    trix = np.ndarray
)
```
	
#### Williams Accumulation/Distribution
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [wad.cc](https://github.com/3jane/tindicators/indicators/wad.cc) <br/>
*Signature:*

```python
ti.wad(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    wad = np.ndarray
)
```
	
#### Williams %R
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [willr.cc](https://github.com/3jane/tindicators/indicators/willr.cc) <br/>
*Signature:*

```python
ti.willr(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    willr = np.ndarray
)
```
	
#### Ultimate Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [ultosc.cc](https://github.com/3jane/tindicators/indicators/ultosc.cc) <br/>
*Signature:*

```python
ti.ultosc(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    short period,
    medium period,
    long period
) -> NamedTuple(...,
    ultosc = np.ndarray
)
```
	
#### Vertical Horizontal Filter
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [vhf.cc](https://github.com/3jane/tindicators/indicators/vhf.cc) <br/>
*Signature:*

```python
ti.vhf(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    vhf = np.ndarray
)
```
	
#### Volume Oscillator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [vosc.cc](https://github.com/3jane/tindicators/indicators/vosc.cc) <br/>
*Signature:*

```python
ti.vosc(
    volume: np.ndarray,
    short period,
    long period
) -> NamedTuple(...,
    vosc = np.ndarray
)
```
	
#### Efficiency Ratio
*Source:* Kaufman. Trading Systems and Methods <br/>
*Implementation:* [er.cc](https://github.com/3jane/tindicators/indicators/er.cc) <br/>
*Signature:*

```python
ti.er(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    er = np.ndarray
)
```
	
#### Price Volume Trend
*Source:* Buff Pelz Dormeier - Investing with Volume Analysis_ Identify, Follow, and Profit from Trends-FT Press (2011) <br/>
*Implementation:* [pvt.cc](https://github.com/3jane/tindicators/indicators/pvt.cc) <br/>
*Signature:*

```python
ti.pvt(
    close: np.ndarray,
    volume: np.ndarray
) -> NamedTuple(...,
    pvt = np.ndarray
)
```
	
#### Hurst Exponent Indicator
*Source:* Stocks & Commodities V. 25:3 (36-42): Trading Systems And Fractals by Radha Panini <br/>
*Implementation:* [hurst.cc](https://github.com/3jane/tindicators/indicators/hurst.cc) <br/>
*Signature:*

```python
ti.hurst(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    hurst = np.ndarray,
    fractal_dim = np.ndarray
)
```
	
#### Adaptive Relative Strength Index
*Source:* Ehlers. Rocket Science for Traders, pp. 229-231 <br/>
*Implementation:* [arsi.cc](https://github.com/3jane/tindicators/indicators/arsi.cc) <br/>
*Signature:*

```python
ti.arsi(
    series: np.ndarray,
    cycpart
) -> NamedTuple(...,
    arsi = np.ndarray
)
```
	
#### Directional Movement
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [dm.cc](https://github.com/3jane/tindicators/indicators/dm.cc) <br/>
*Signature:*

```python
ti.dm(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    plus_dm = np.ndarray,
    minus_dm = np.ndarray
)
```
	
#### Directional Movement Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [dx.cc](https://github.com/3jane/tindicators/indicators/dx.cc) <br/>
*Signature:*

```python
ti.dx(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    dx = np.ndarray
)
```
	
#### Directional Indicator
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [di.cc](https://github.com/3jane/tindicators/indicators/di.cc) <br/>
*Signature:*

```python
ti.di(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    plus_di = np.ndarray,
    minus_di = np.ndarray
)
```
	
#### Average Directional Movement Index
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [adx.cc](https://github.com/3jane/tindicators/indicators/adx.cc) <br/>
*Signature:*

```python
ti.adx(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    adx = np.ndarray
)
```
	
#### Average Directional Movement Rating
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [adxr.cc](https://github.com/3jane/tindicators/indicators/adxr.cc) <br/>
*Signature:*

```python
ti.adxr(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    adxr = np.ndarray
)
```
	
#### Mesa Sine Wave
*Source:* - <br/>
*Implementation:* [msw.cc](https://github.com/3jane/tindicators/indicators/msw.cc) <br/>
*Signature:*

```python
ti.msw(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    msw_sine = np.ndarray,
    msw_lead = np.ndarray
)
```
	
#### Momentum
*Source:* - <br/>
*Implementation:* [mom.cc](https://github.com/3jane/tindicators/indicators/mom.cc) <br/>
*Signature:*

```python
ti.mom(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    mom = np.ndarray
)
```
	
#### Rate of Change
*Source:* - <br/>
*Implementation:* [roc.cc](https://github.com/3jane/tindicators/indicators/roc.cc) <br/>
*Signature:*

```python
ti.roc(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    roc = np.ndarray
)
```
	
#### Rate of Change Ratio
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [rocr.cc](https://github.com/3jane/tindicators/indicators/rocr.cc) <br/>
*Signature:*

```python
ti.rocr(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    rocr = np.ndarray
)
```
	
#### Lag
*Source:* - <br/>
*Implementation:* [lag.cc](https://github.com/3jane/tindicators/indicators/lag.cc) <br/>
*Signature:*

```python
ti.lag(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    lag = np.ndarray
)
```
	
#### Maximum In Period
*Source:* - <br/>
*Implementation:* [max.cc](https://github.com/3jane/tindicators/indicators/max.cc) <br/>
*Signature:*

```python
ti.max(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    max = np.ndarray
)
```
	
#### Minimum In Period
*Source:* - <br/>
*Implementation:* [min.cc](https://github.com/3jane/tindicators/indicators/min.cc) <br/>
*Signature:*

```python
ti.min(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    min = np.ndarray
)
```
	
#### Sum Over Period
*Source:* - <br/>
*Implementation:* [sum.cc](https://github.com/3jane/tindicators/indicators/sum.cc) <br/>
*Signature:*

```python
ti.sum(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    sum = np.ndarray
)
```
	
#### Standard Deviation Over Period
*Source:* - <br/>
*Implementation:* [stddev.cc](https://github.com/3jane/tindicators/indicators/stddev.cc) <br/>
*Signature:*

```python
ti.stddev(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    stddev = np.ndarray
)
```
	
#### Standard Error Over Period
*Source:* - <br/>
*Implementation:* [stderr.cc](https://github.com/3jane/tindicators/indicators/stderr.cc) <br/>
*Signature:*

```python
ti.stderr(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    stderr = np.ndarray
)
```
	
#### Mean Deviation Over Period
*Source:* - <br/>
*Implementation:* [md.cc](https://github.com/3jane/tindicators/indicators/md.cc) <br/>
*Signature:*

```python
ti.md(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    md = np.ndarray
)
```
	
#### Variance Over Period
*Source:* - <br/>
*Implementation:* [var.cc](https://github.com/3jane/tindicators/indicators/var.cc) <br/>
*Signature:*

```python
ti.var(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    var = np.ndarray
)
```
	
#### Average Price
*Source:* - <br/>
*Implementation:* [avgprice.cc](https://github.com/3jane/tindicators/indicators/avgprice.cc) <br/>
*Signature:*

```python
ti.avgprice(
    open: np.ndarray,
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    avgprice = np.ndarray
)
```
	
#### Median Price
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [medprice.cc](https://github.com/3jane/tindicators/indicators/medprice.cc) <br/>
*Signature:*

```python
ti.medprice(
    high: np.ndarray,
    low: np.ndarray
) -> NamedTuple(...,
    medprice = np.ndarray
)
```
	
#### Typical Price
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [typprice.cc](https://github.com/3jane/tindicators/indicators/typprice.cc) <br/>
*Signature:*

```python
ti.typprice(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    typprice = np.ndarray
)
```
	
#### Weighted Close Price
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [wcprice.cc](https://github.com/3jane/tindicators/indicators/wcprice.cc) <br/>
*Signature:*

```python
ti.wcprice(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    wcprice = np.ndarray
)
```
	
#### Average True Range
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [atr.cc](https://github.com/3jane/tindicators/indicators/atr.cc) <br/>
*Signature:*

```python
ti.atr(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    atr = np.ndarray
)
```
	
#### Chandelier Exit
*Source:* J. Welles Wilder. New Concepts in Technical Trading Systems, 1978, pp. 21-23 <br/>
*Implementation:* [ce.cc](https://github.com/3jane/tindicators/indicators/ce.cc) <br/>
*Signature:*

```python
ti.ce(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period,
    coef
) -> NamedTuple(...,
    ce_high = np.ndarray,
    ce_low = np.ndarray
)
```
	
#### Normalized Average True Range
*Source:* - <br/>
*Implementation:* [natr.cc](https://github.com/3jane/tindicators/indicators/natr.cc) <br/>
*Signature:*

```python
ti.natr(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    natr = np.ndarray
)
```
	
#### True Range
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [tr.cc](https://github.com/3jane/tindicators/indicators/tr.cc) <br/>
*Signature:*

```python
ti.tr(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray
) -> NamedTuple(...,
    tr = np.ndarray
)
```
	
#### Ichimoku
*Source:* Trading with Ichimoku Clouds <br/>
*Implementation:* [ichi.cc](https://github.com/3jane/tindicators/indicators/ichi.cc) <br/>
*Signature:*

```python
ti.ichi(
    high: np.ndarray,
    low: np.ndarray,
    period9,
    period26,
    period52
) -> NamedTuple(...,
    ichi_tenkan_sen = np.ndarray,
    ichi_kijun_sen = np.ndarray,
    ichi_senkou_span_A = np.ndarray,
    ichi_senkou_span_B = np.ndarray
)
```
	
#### Annualized Historical Volatility
*Source:* - <br/>
*Implementation:* [volatility.cc](https://github.com/3jane/tindicators/indicators/volatility.cc) <br/>
*Signature:*

```python
ti.volatility(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    volatility = np.ndarray
)
```
	
#### Chaikins Volatility
*Source:* Technical Analysis from A to Z <br/>
*Implementation:* [cvi.cc](https://github.com/3jane/tindicators/indicators/cvi.cc) <br/>
*Signature:*

```python
ti.cvi(
    high: np.ndarray,
    low: np.ndarray,
    period
) -> NamedTuple(...,
    cvi = np.ndarray
)
```
	
#### Crossany
*Source:* - <br/>
*Implementation:* [crossany.cc](https://github.com/3jane/tindicators/indicators/crossany.cc) <br/>
*Signature:*

```python
ti.crossany(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    crossany = np.ndarray
)
```
	
#### Crossover
*Source:* - <br/>
*Implementation:* [crossover.cc](https://github.com/3jane/tindicators/indicators/crossover.cc) <br/>
*Signature:*

```python
ti.crossover(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    crossover = np.ndarray
)
```
	
#### Linear Decay
*Source:* - <br/>
*Implementation:* [decay.cc](https://github.com/3jane/tindicators/indicators/decay.cc) <br/>
*Signature:*

```python
ti.decay(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    decay = np.ndarray
)
```
	
#### Exponential Decay
*Source:* - <br/>
*Implementation:* [edecay.cc](https://github.com/3jane/tindicators/indicators/edecay.cc) <br/>
*Signature:*

```python
ti.edecay(
    series: np.ndarray,
    period
) -> NamedTuple(...,
    edecay = np.ndarray
)
```
	
#### Vector Addition
*Source:* - <br/>
*Implementation:* [add.cc](https://github.com/3jane/tindicators/indicators/add.cc) <br/>
*Signature:*

```python
ti.add(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    add = np.ndarray
)
```
	
#### Vector Subtraction
*Source:* - <br/>
*Implementation:* [sub.cc](https://github.com/3jane/tindicators/indicators/sub.cc) <br/>
*Signature:*

```python
ti.sub(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    sub = np.ndarray
)
```
	
#### Vector Multiplication
*Source:* - <br/>
*Implementation:* [mul.cc](https://github.com/3jane/tindicators/indicators/mul.cc) <br/>
*Signature:*

```python
ti.mul(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    mul = np.ndarray
)
```
	
#### Vector Division
*Source:* - <br/>
*Implementation:* [div.cc](https://github.com/3jane/tindicators/indicators/div.cc) <br/>
*Signature:*

```python
ti.div(
    series: np.ndarray,
    series: np.ndarray
) -> NamedTuple(...,
    div = np.ndarray
)
```
	
#### Vector Absolute Value
*Source:* - <br/>
*Implementation:* [abs.cc](https://github.com/3jane/tindicators/indicators/abs.cc) <br/>
*Signature:*

```python
ti.abs(
    series: np.ndarray
) -> NamedTuple(...,
    abs = np.ndarray
)
```
	
#### Vector Arccosine
*Source:* - <br/>
*Implementation:* [acos.cc](https://github.com/3jane/tindicators/indicators/acos.cc) <br/>
*Signature:*

```python
ti.acos(
    series: np.ndarray
) -> NamedTuple(...,
    acos = np.ndarray
)
```
	
#### Vector Arcsine
*Source:* - <br/>
*Implementation:* [asin.cc](https://github.com/3jane/tindicators/indicators/asin.cc) <br/>
*Signature:*

```python
ti.asin(
    series: np.ndarray
) -> NamedTuple(...,
    asin = np.ndarray
)
```
	
#### Vector Arctangent
*Source:* - <br/>
*Implementation:* [atan.cc](https://github.com/3jane/tindicators/indicators/atan.cc) <br/>
*Signature:*

```python
ti.atan(
    series: np.ndarray
) -> NamedTuple(...,
    atan = np.ndarray
)
```
	
#### Vector Ceiling
*Source:* - <br/>
*Implementation:* [ceil.cc](https://github.com/3jane/tindicators/indicators/ceil.cc) <br/>
*Signature:*

```python
ti.ceil(
    series: np.ndarray
) -> NamedTuple(...,
    ceil = np.ndarray
)
```
	
#### Vector Cosine
*Source:* - <br/>
*Implementation:* [cos.cc](https://github.com/3jane/tindicators/indicators/cos.cc) <br/>
*Signature:*

```python
ti.cos(
    series: np.ndarray
) -> NamedTuple(...,
    cos = np.ndarray
)
```
	
#### Vector Hyperbolic Cosine
*Source:* - <br/>
*Implementation:* [cosh.cc](https://github.com/3jane/tindicators/indicators/cosh.cc) <br/>
*Signature:*

```python
ti.cosh(
    series: np.ndarray
) -> NamedTuple(...,
    cosh = np.ndarray
)
```
	
#### Vector Exponential
*Source:* - <br/>
*Implementation:* [exp.cc](https://github.com/3jane/tindicators/indicators/exp.cc) <br/>
*Signature:*

```python
ti.exp(
    series: np.ndarray
) -> NamedTuple(...,
    exp = np.ndarray
)
```
	
#### Vector Floor
*Source:* - <br/>
*Implementation:* [floor.cc](https://github.com/3jane/tindicators/indicators/floor.cc) <br/>
*Signature:*

```python
ti.floor(
    series: np.ndarray
) -> NamedTuple(...,
    floor = np.ndarray
)
```
	
#### Vector Natural Log
*Source:* - <br/>
*Implementation:* [ln.cc](https://github.com/3jane/tindicators/indicators/ln.cc) <br/>
*Signature:*

```python
ti.ln(
    series: np.ndarray
) -> NamedTuple(...,
    ln = np.ndarray
)
```
	
#### Vector Base-10 Log
*Source:* - <br/>
*Implementation:* [log10.cc](https://github.com/3jane/tindicators/indicators/log10.cc) <br/>
*Signature:*

```python
ti.log10(
    series: np.ndarray
) -> NamedTuple(...,
    log10 = np.ndarray
)
```
	
#### Vector Round
*Source:* - <br/>
*Implementation:* [round.cc](https://github.com/3jane/tindicators/indicators/round.cc) <br/>
*Signature:*

```python
ti.round(
    series: np.ndarray
) -> NamedTuple(...,
    round = np.ndarray
)
```
	
#### Vector Sine
*Source:* - <br/>
*Implementation:* [sin.cc](https://github.com/3jane/tindicators/indicators/sin.cc) <br/>
*Signature:*

```python
ti.sin(
    series: np.ndarray
) -> NamedTuple(...,
    sin = np.ndarray
)
```
	
#### Vector Hyperbolic Sine
*Source:* - <br/>
*Implementation:* [sinh.cc](https://github.com/3jane/tindicators/indicators/sinh.cc) <br/>
*Signature:*

```python
ti.sinh(
    series: np.ndarray
) -> NamedTuple(...,
    sinh = np.ndarray
)
```
	
#### Vector Square Root
*Source:* - <br/>
*Implementation:* [sqrt.cc](https://github.com/3jane/tindicators/indicators/sqrt.cc) <br/>
*Signature:*

```python
ti.sqrt(
    series: np.ndarray
) -> NamedTuple(...,
    sqrt = np.ndarray
)
```
	
#### Vector Tangent
*Source:* - <br/>
*Implementation:* [tan.cc](https://github.com/3jane/tindicators/indicators/tan.cc) <br/>
*Signature:*

```python
ti.tan(
    series: np.ndarray
) -> NamedTuple(...,
    tan = np.ndarray
)
```
	
#### Vector Hyperbolic Tangent
*Source:* - <br/>
*Implementation:* [tanh.cc](https://github.com/3jane/tindicators/indicators/tanh.cc) <br/>
*Signature:*

```python
ti.tanh(
    series: np.ndarray
) -> NamedTuple(...,
    tanh = np.ndarray
)
```
	
#### Vector Degree Conversion
*Source:* - <br/>
*Implementation:* [todeg.cc](https://github.com/3jane/tindicators/indicators/todeg.cc) <br/>
*Signature:*

```python
ti.todeg(
    series: np.ndarray
) -> NamedTuple(...,
    degrees = np.ndarray
)
```
	
#### Vector Radian Conversion
*Source:* - <br/>
*Implementation:* [torad.cc](https://github.com/3jane/tindicators/indicators/torad.cc) <br/>
*Signature:*

```python
ti.torad(
    series: np.ndarray
) -> NamedTuple(...,
    radians = np.ndarray
)
```
	
#### Vector Truncate
*Source:* - <br/>
*Implementation:* [trunc.cc](https://github.com/3jane/tindicators/indicators/trunc.cc) <br/>
*Signature:*

```python
ti.trunc(
    series: np.ndarray
) -> NamedTuple(...,
    trunc = np.ndarray
)
```
	
#### Vortex Indicator
*Source:* The Vortex Indicator <br/>
*Implementation:* [vi.cc](https://github.com/3jane/tindicators/indicators/vi.cc) <br/>
*Signature:*

```python
ti.vi(
    high: np.ndarray,
    low: np.ndarray,
    close: np.ndarray,
    period
) -> NamedTuple(...,
    vi_p = np.ndarray,
    vi_m = np.ndarray
)
```
	