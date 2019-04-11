import pytest

import pytulip
import numpy as np


indicators = pytulip.TulipIndicators()

f = indicators.fisher
result = f(np.array([1,2,3]), np.array([1,2,3]), 1)

# run: python3 -i test.py 