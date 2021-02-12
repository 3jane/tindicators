# tindicators
Blazing fast Python library of 160+ technical analysis indicators.

Install: `pip install tindicators`

Docs: [https://3jane.github.io/tindicators](https://3jane.github.io/tindicators)

## Usage
```python3
>>> from tindicators import ti
>>> ti
tindicators, version 0.9.11
161 indicators are available: abands abs acos ad add adosc adx adxr ahma ao apo aroon aroonosc arsi asin atan atr avgprice bbands bf2 bf3 bop cci ce ceil cmf cmi cmo copp cos cosh crossany crossover cvi decay dema di div dm dpo dwma dx edcf edecay ehma ema emsd emv er evwma exp fi fisher floor fosc frama gf1 gf2 gf3 gf4 hd hf hfema hfsma hma hurst hwma ichi idwma kama kc kst kvo lag lf linreg linregintercept linregslope lma ln log10 macd mama marketfi mass max md medprice mesastoch mfi mgdyn mhlma min mom msw mul natr nvi obv pbands pc pfe posc ppo psar pvi pvt pwma qstick rema rmf rmi rmta roc rocr roof round rsi rvi shmma sin sinh sma smi sqrt ssmooth stddev stderr stoch stochrsi sub sum swma t3 tan tanh tema todeg torad tr trima trix trunc tsf tsi typprice ultosc var vhf vi vidya volatility vosc vwap vwma wad wcprice wilders willr wma zlema
>>> ti.pbands
Name:     	pbands
Full Name:	Projection Bands
Inputs:   	high low close
Options:  	period
Outputs:  	pbands_lower pbands_upper
>>> ti.pbands([207.75,204.94,204.15,203.38,201.37,199.85],[203.90,202.34,202.52,198.61,198.56,198.01],[207.48,204.53,203.86,203.13,199.25,199.23], 3)
pbands_3(pbands_lower=array([    nan,     nan, 200.28 , 198.61 , 196.305, 194.71 ]), pbands_upper=array([   nan,    nan, 204.15, 203.54, 201.37, 199.85]))
>>> ti.ema
Name:     	ema
Full Name:	Exponential Moving Average
Inputs:   	series
Options:  	period
Outputs:  	ema
>>> ti.ema([1,2,3,4,5], 4)
array([1.    , 1.4   , 2.04  , 2.824 , 3.6944])
```
