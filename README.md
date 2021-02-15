# tindicators

**tindicators** is a library of technical analysis indicators. It 
provides over 160 indicators and has interfaces for C, Python and C#. It is blazing fast.

### Quick start
```bash
pip install tindicators
```
```python
from tindicators import ti
```
Example usage:

```python
>>> ti.ema
Name:     	ema
Full Name:	Exponential Moving Average
Inputs:   	series
Options:  	period
Outputs:  	ema
>>> ti.ema([1,2,3,4,5], 4)
array([1.    , 1.4   , 2.04  , 2.824 , 3.6944])
```

Docs: https://3jane.github.io/tindicators/

### More info

See also the [C# package](bindings/lean).

The goal of this project was to create a library of indicators that would be fast, complete and easy to integrate into other systems. It is based on a fork of [tulipindicators](http://github.com/tulipcharts/tulipindicators). Design overview and how to contribute: [CONTRIBUTING.md](./CONTRIBUTING.md)