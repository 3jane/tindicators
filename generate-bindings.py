from tulipindicators import ti
import os
import shutil

# - base class `indicator' that implements run, stream_run etc on managed arrays
# - successors that have static fields "name" etc and implement run_raw, stream_run_raw, etc
# - the only method will need to be changed: https://github.com/hcmc-project/tulipindicators-net-lean/blob/STR-172-indicators-wrapper/TulipIndicators.cs#L13

tmpl_toplevel = '''\
using System;
using System.Runtime.InteropServices;
using QuantConnect.Data.Market;
using QuantConnect.Indicators;

namespace Rcdb.TulipIndicators {
    $util
    $streaming
    $default
}
'''

util = '''\
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
'''

tmpl_streaming = '''\
namespace Streaming {
    $indicators
}
'''

tmpl_default = '''\
namespace Default {
    $indicators
}
'''


dllimport = '\n'.join([
    '[DllImport("indicators", EntryPoint="{fun}")]',
    'static extern {ret} {fun}({args});'
])

def streaming(indicator):
    inputs = indicator.inputs
    outputs = indicator.outputs
    name = indicator.name
    options = indicator.options

    n = '\n'
    real = 'real' in inputs
    if real and len(inputs) > 1:
        print(f"warning: skipping '{name}' for its inputs: {inputs}")
        return ''

    input_type = ['TradeBar', 'IndicatorDataPoint'][real]

    result = f'''
    public class {name} : IndicatorBase<{input_type}> {{
        IntPtr state;
        {f'{n}'.join(f'public Identity {output.capitalize()};' for output in outputs)}
        public {name}({', '.join(map('double {}'.format, options))}) : base("{name}") {{
            int ret = ti_{name}_stream_new(new double[]{{{', '.join(options)}}}, ref state);
            util.DispatchError(ret);
            {f'{n}'.join(f'{output.capitalize()} = new Identity("{output}");' for output in outputs)}
        }}
        protected override decimal ComputeNextValue({input_type} data) {{
            IntPtr[] inputs = new IntPtr[{len(inputs)}];
            IntPtr[] outputs = new IntPtr[{len(outputs)}];
            double[] tmp = new double[1];
            {f"{n}".join(f'inputs[{i}] = Marshal.AllocHGlobal(1);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'tmp[0] = (double)data.{input.capitalize() if not real else "Value"};' for i, input in enumerate(inputs))}
            {f"{n}".join(f'Marshal.Copy(tmp, 0, inputs[{i}], 1);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'outputs[{i}] = Marshal.AllocHGlobal(1);' for i, output in enumerate(outputs))}
            int result = ti_{name}_stream_run(state, 1, inputs, outputs);
            util.DispatchError(result);
            {f"{n}".join(f'Marshal.Copy(outputs[{i}], tmp, 0, 1); {output.capitalize()}.Update(data.Time, (decimal)tmp[0]);' for i, output in enumerate(outputs))}
            foreach (IntPtr input in inputs) {{ Marshal.FreeHGlobal(input); }}
            foreach (IntPtr output in outputs) {{ Marshal.FreeHGlobal(output); }}
            return (decimal){outputs[0].capitalize()}.Current.Value;
        }}
        public override bool IsReady {{
            get {{ return ti_stream_get_progress(state) > 0; }}
        }}
        ~{name}() {{ ti_{name}_stream_free(state); }}
        {dllimport.format(fun=f'ti_{name}_stream_free', ret='void', args='IntPtr state')}
        {dllimport.format(fun='ti_stream_get_progress', ret='int', args='IntPtr state')}
        {dllimport.format(fun=f'ti_{name}_stream_new', ret='int', args='double[] options, ref IntPtr state')}
        {dllimport.format(fun=f'ti_{name}_stream_run', ret='int', args='IntPtr state, int size, IntPtr[] inputs, IntPtr[] outputs')}
    }}
    '''
    return result

if __name__ == '__main__':
    srclibpath = ti._lib._name
    dirpath = os.path.dirname(os.path.abspath(__file__))
    shutil.copy(srclibpath, dirpath)

    result = tmpl_toplevel \
        .replace('$util', util) \
        .replace('$streaming', '\n'.join(streaming(ti.__getattr__(name).info) for name in ['pbands'])) \
        .replace('$default', '')

    with open('tulipindicators.cs', 'w') as f:
        f.write(result)
