# tulipindicators-python
Python wrapper for [tulipindicators-private](https://github.com/hcmc-project/tulipindicators-private)

## Installation
1. Copy the contents of [this file](pip.conf) to `~/.config/pip/pip.conf` (on Linux) or `%APPDATA%\pip\pip.ini` (on Windows)
2. `pip install tulipindicators`

## Usage
```python
>>> from tulipindicators import ti
>>> ti
Tulip Indicators, version 0.9.8
123 indicators are available: abands abs acos ad add adosc adx adxr ao apo aroon aroonosc asin atan atr avgprice bbands bop cci ce ceil cmf cmo copp cos cosh crossany crossover cvi dc decay dema di div dm dpo dx edecay ema emv exp fi fisher floor fosc frama hma kama kc kst kvo lag linreg linregintercept linregslope ln log10 macd marketfi mass max md medprice mfi min mom msw mul natr nvi obv pbands pc pfe posc ppo psar pvi qstick rmi rmta roc rocr round rsi rvi sin sinh sma smi sqrt stddev stderr stoch stochrsi sub sum tan tanh tema todeg torad tr trima trix trunc tsf tsi typprice ultosc var vhf vidya volatility vosc vwap vwma wad wcprice wilders willr wma zlema
>>> ti.pbands
Name:           pbands
Full Name:      Projection Bands
Inputs:         high low close
Options:        period
Outputs:        pbands_lower pbands_upper
>>> ti.pbands([207.75,204.94,204.15,203.38,201.37,199.85],[203.90,202.34,202.52,198.61,198.56,198.01],[207.48,204.53,203.86,203.13,199.25,199.23], 3)
pbands_3(pbands_lower=array([    nan,     nan, 200.28 , 198.61 , 196.305, 194.71 ]), pbands_upper=array([   nan,    nan, 204.15, 203.54, 201.37, 199.85]))
>>> ti.ema
Name:           ema
Full Name:      Exponential Moving Average
Inputs:         real
Options:        period
Outputs:        ema
>>> ti.ema([1,2,3,4,5], 4)
array([1.    , 1.4   , 2.04  , 2.824 , 3.6944])
```