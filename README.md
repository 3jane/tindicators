## tulipindicators-net

1. download `tulipindicators-X.Y.zip` from [here](https://github.com/hcmc-project/tulipindicators-python/releases)
2. run `pip install` on it
3. run `generate-bindings.py`

Interop file `tulipindicators.cs` will be generated. Copy it together with the library binaries to where needed. (We have yet to setup packaging.)

--------------------
```
indicators are divided into two kinds:
- ohlc (: IndicatorBase<IBaseDataBar>) (: IndicatorBase<TradeBar>)
- real (: IndicatorBase<IndicatorDataPoint>)

indicator always has a plain interface and sometimes streaming one

indicator implementation must override the following functions:

    TulipIndicatorBase.cs:
        ComputeNextValue(ReverseRollingWindow<T> window, T input)

    ReverseWindowIndicator.cs:
        IsReady
        ComputeNextValue(T input)
        Reset()

    TulipTradeBarIndicator.cs:
        [NOT] Validate()
        [NOT] TakeInputFrom(TradeBar bar, string inputName)

    TulipDataPointIndicator.cs:
        [NOT] TakeInputFrom(IndicatorDataPoint dataPoint, string inputName)
        [NOT] Validate()
        [NOT] UniversalUpdate(TradeBar bar)

    TulipOutputIndicator.cs:
        IsReady => true;
        ComputeNextValue(IndicatorDataPoint input) => input.Value;

so, there are three essential parts:
1. ComputeNextValue
2. IsReady
3. Outputs as identity indicators

so the classes should look like this:

    namespace TulipIndicators {
        class util {
            public class InvalidOption : System.Exception {}
            public class OutOfMemory : System.Exception {}
            public static void DispatchError(int ret) {
                switch(ret) {
                    case 0: break;
                    case 1: throw new InvalidOption();
                    case 2: throw new OutOfMemory();
                    default: throw new System.Exception();
                }
            }
        }
        namespace Streaming {
            class sma : IndicatorBase<IndicatorDataPoint> {
                IntPtr state;
                public sma(options) { util.DispatchException(ti_sma_stream_new(options, ref state)); }
                public override decimal ComputeNextValue(IndicatorDataPoint data) {
                    ...
                }
                public override bool IsReady = ti_stream_get_progress(state) > 0;
                ~sma() { ti_sma_stream_free(state); }
            }
            class abands : IndicatorBase<TradeBar> {
                IntPtr state;
                public sma(options) { util.DispatchException(ti_sma_stream_new(options, ref state)); }
                public override decimal ComputeNextValue(TradeBar data) {
                    ...
                }
                public override bool IsReady = ti_stream_get_progress(state) > 0;
                ~abands() { ti_abands_stream_free(state); }
            }
        }
        namespace Default {
            class ema : WindowIndicator<IndicatorDataPoint> {
                double[] options;
                double period;
                public ema(options, insize) {  }
                public decimal ComputeNextValue(RollingWindow<IndicatorDataPoint> window, IndicatorDataPoint data) {
                    ...
                }
                public override bool IsReady = window.Count >= period;
            }
            class pbands : WindowIndicator<TradeBar> {
                double[] options;
                double period;
                public Identity Lower;
                public Identity Upper;
                public pbands(options, insize) { }
                public decimal ComputeNextValue(RollingWindow<TradeBar> window, TradeBar data) {
                    ...
                }
                public override bool IsReady = window.Count >= period;
            }
        }
    }

error codes must be transformed into exceptions
```