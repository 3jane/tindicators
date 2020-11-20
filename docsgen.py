import yaml
import time
import os
import argparse
import glob

from version import version

parser = argparse.ArgumentParser(description='Generate the docs for Tulip Indicators')
# parser.add_argument('--old', help='use old defaults', action='store_true')
parser.add_argument('outfile', default='index.markdown')
args = parser.parse_args()


build = int(time.time())

path_prefix = os.path.join(os.path.dirname(os.path.realpath(__file__)), '')

indicators = yaml.safe_load(open(path_prefix+'indicators.yaml', encoding='utf8'))

outfile = open(path_prefix+args.outfile, 'w')

outfile.write('''\
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
There are currently **{num} indicators** available in [tindicators](https://github.com/3jane/tindicators) v{version}.
'''.format(num=len(indicators), version=version))

for indicator in indicators.items():
	name, (elab_name, type, inputs, options, outputs, features, source) = indicator

	# inputs_str = ', '.join([x+': np.ndarray' for x in inputs])
	# options_str = ', '.join([x+': float' for x in options])
	# params_str = ', '.join([inputs_str, options_str])

	# outputs_str = ', '.join([x+'=np.ndarray' for x in outputs])

# 	outfile.write(f'''
# ### {elab_name}
# Source: {source}

# Signature:
# ```python
# ti.{name}({params_str}) ->
#     NamedTuple(_, {outputs_str})
# ```
# 	''')

	tab = ' '*4
	nl = '\n'

	inputs_str = f',{nl}{tab}'.join([x+': np.ndarray' for x in inputs])
	options_str = f',{nl}{tab}'.join([x+'' for x in options])
	params_str = f',{nl}{tab}'.join([inputs_str, options_str]) \
		if (inputs_str and options_str) \
		else (inputs_str or options_str)

	outputs_str = f',{nl}{tab}'.join([x+' = np.ndarray' for x in outputs])

	filename = os.path.basename(list(glob.glob(path_prefix+f'indicators/{name}.*'))[0])

	outfile.write(f'''
#### {elab_name}
*Source:* {source} <br/>
*Implementation:* [{filename}](https://github.com/3jane/tindicators/indicators/{filename}) <br/>
*Signature:*

```python
ti.{name}(
    {params_str}
) -> NamedTuple(...,
    {outputs_str}
)
```
	''')


# 	outputs_str = f',{nl}{tab}'.join([f"'{x}': np.ndarray" for x in outputs])

# 	outfile.write(f'''
# ### {elab_name}
# Source: {source}

# Signature:
# ```python
# ti.{name}(
#     {params_str}
# ) -> NamedTuple(_, **{{
#     {outputs_str}
# }})
# ```
# 	''')
