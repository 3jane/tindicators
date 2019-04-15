## Here is my little research on this matter

### CppSharp
- by default, a class with static methods is generated
- depending on the data in the function table, generate
	1) the correct constructor parameters
	2) the correct outputs
	3) if streaming:
		1. ctor that calls the _stream_new and keeps the pointer in a field
		2. dtor that calls the _stream_free
		3. GetNextValue() that calls to _stream_run and takes a single bar or number
	4) else:
	   	1. ctor that only keeps the options + specifies how large the window should be
	   	2. GetNextValue() that calls to the indicator and takes a window of bars or numbers

### selection of input data:
1) each of ohlcv is connected to its input
2) each real is connected by mean of its corresponding selector

the translation tradebar -> datapoint is performed by RegisterIndicator by means of a selector,
and in case we want to support an indicator that takes a real together with something else,
we will probably need to create custom class a-la `TIInputBar` that would contain
source: https://www.quantconnect.com/docs/algorithm-reference/indicators

-------------

### conclusion:
1) generate the simplest binding from the indicators.h by means of CppSharp
2) all rest is to be hooked up by C#'s reflection with delegates
