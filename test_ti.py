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
