import pytest
import numpy as np
import pandas as pd

from tulipindicators import TulipIndicators


REAL = np.array([81.59, 81.06, 82.87, 83,    83.61,
                  83.15, 82.84, 83.99, 84.55, 84.36,
                  85.53, 86.54, 86.89, 87.77, 87.29])
EXPECTED = np.array([82.426, 82.738, 83.094, 83.318, 83.628,
                     83.778, 84.254, 84.994, 85.574, 86.218,
                     86.804]),

ti = TulipIndicators()


def test_sma():
    result = ti.sma(REAL, 5, pad_left=False)
    assert np.allclose(result, EXPECTED)


def test_sma_accept_series():
    series_real = pd.Series(REAL)
    result = ti.sma(series_real, 5, pad_left=False)
    assert np.allclose(result, EXPECTED)


def test_pad_left():
    series_real = pd.Series(REAL)
    result = ti.sma(series_real, 5)

    assert len(result[0]) == len(REAL)


def test_indicator_info():
    assert ti.bbands.info.full_name == 'Bollinger Bands'
    assert ti.bbands.info.name == 'bbands'
    assert ti.bbands.info.type == 'overlay'
    assert ti.bbands.info.inputs == ('real',)
    assert ti.bbands.info.options == ('period', 'stddev')
    assert ti.bbands.info.outputs == (
        'bbands_lower', 'bbands_middle', 'bbands_upper')


def test_convert_to_dataframe():
    ohlc = pd.DataFrame({
        'Open': np.array([80, 81.1, 83.1]),
        'High': np.array([84.3, 85.5, 89.7]),
        'Low': np.array([79.7, 76.0, 70.0]),
        'Close': np.array([81.1, 83.1, 85.0])
    })
    delnan = lambda x: x[~np.isnan(x)]
    bbands = ti.bbands(ohlc.Close, period=3, stddev=2)
    ohlc['lower'], ohlc['middle'], ohlc['upper'] = bbands
    assert np.allclose(delnan(ohlc['lower'].values), 
                       delnan(bbands.bbands_lower))
    assert np.allclose(delnan(ohlc['middle'].values),
                       delnan(bbands.bbands_middle))
    assert np.allclose(delnan(ohlc['upper'].values),
                       delnan(bbands.bbands_upper))


def test_bop():
#     print(ti.bop)
    ohlc = pd.DataFrame({
        'Open': np.array([81.85, 81.2, 81.55, 82.91, 83.1, 83.41, 82.71, 82.7, 84.2, 84.25, 84.03, 85.45]),
        'High': np.array([82.15, 81.89, 83.03, 83.3, 83.85, 83.9, 83.33, 84.3, 84.84, 85, 85.9, 86.58]),
        'Low': np.array([81.29, 80.64, 81.31, 82.65, 83.07, 83.11, 82.49, 82.3, 84.15, 84.11, 84.03, 85.39]),
        'Close': np.array([81.59, 81.06, 82.87, 83, 83.61, 83.15, 82.84, 83.99, 84.55, 84.36, 85.53, 86.54])
    })
    expected = np.array([-0.3023, -0.112, 0.7674, 0.1385, 0.6538, -0.3291, 
                         0.1548, 0.645, 0.5072, 0.1236, 0.8021, 0.916])
    bop = ti.bop(ohlc.Open, ohlc.High, ohlc.Low, ohlc.Close, pad_left=False)
    print(bop)
    assert np.allclose(bop, expected) 

test_bop()