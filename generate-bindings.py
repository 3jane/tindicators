from tulipindicators import ti
import os
import shutil

# - base class `indicator' that implements run, stream_run etc on managed arrays
# - successors that have static fields "name" etc and implement run_raw, stream_run_raw, etc
# - the only method will need to be changed: https://github.com/hcmc-project/tulipindicators-net-lean/blob/STR-172-indicators-wrapper/TulipIndicators.cs#L13

tmpl_toplevel = '''\
using System;
using System.Runtime.InteropServices;

namespace TulipIndicators {
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

def tradebar(indicator):
    inputs = indicator.inputs;
    outputs = indicator.outputs;
    name = indicator.name;
    options = indicator.options;

    result = f'''\
    public class {name} : IndicatorBase<TradeBar> {{
        IntPtr state;
        {'\n'.join(f'public Identity {output.capitalize()};') for output in outputs}
        public {name}({', '.join(map('double {}'.format, options))}) {{
            int ret = ti_{name}_stream_new(new double[]{{{', '.join(options)}}}, ref state);
            util.DispatchException(ret);
            {'\n'.join(f'{output.capitalize()} = new Identity("{output}");') for output in outputs}
        }}
        public override decimal ComputeNextValue(TradeBar data) {{
            IntPtr[] inputs;
            IntPtr[] outputs;
            {"\n".join(f'inputs[{i}] = AllocHGlobal(sizeof(double));' for i, input in enumerate(inputs))}
            {"\n".join(f'Marshal.Copy(data.{input.capitalize()}, 0, inputs[{i}], sizeof(double));' for i, input in enumerate(inputs))}
            {"\n".join(f'outputs[{i}] = AllocHGlobal(sizeof(double));' for i, output in enumerate(outputs))}
            int result = ti_{name}_stream_run(state, 1, inputs, outputs);
            util.DispatchException(result);
            double tmp;
            {"\n".join(f'Marshal.Copy(outputs[{i}], tmp, 0, sizeof(double)); {output.capitalize()}.Update(tmp);' for i, output in enumerate(outputs))}
            foreach (IntPtr input in inputs) {{ FreeHGlobal(input); }}
            foreach (IntPtr output in outputs) {{ FreeHGlobal(output); }}
            return {outputs[0].capitalize()}.Current.Value;
        }}
        public override bool IsReady = ti_stream_get_progress(state) > 0;
        ~{name}() {{ ti_{name}_stream_free(state); }}
        {dllimport.format(fun=f'ti_{name}_stream_free', ret='void', args='IntPtr state')}
        {dllimport.format(fun='ti_stream_get_progress', ret='int', args='IntPtr state')}
        {dllimport.format(fun=f'ti_{name}_stream_new', ret='int', args='double[] options, ref IntPtr state')}
        {dllimport.format(fun=f'ti_{name}_stream_run', ret='int', args='IntPtr state, int size, IntPtr[] inputs, IntPtr[] outputs')}
    }}
    '''
    return result

tmpl_streaming_ind_tradebar = '''\
public class $name : IndicatorBase<TradeBar> {
    IntPtr state;
    public $name($options) { util.DispatchException(ti_$name_stream_new(new double[]{$option_names}, ref state)); }
    public override decimal ComputeNextValue(TradeBar data) {
        ...
    }
    public override bool IsReady = ti_stream_get_progress(state) > 0;
    ~$name() { ti_$name_stream_free(state); }
    $imported
}
'''

def datapoint(indicator):
    inputs = indicator.inputs;
    outputs = indicator.outputs;
    name = indicator.name;
    options = indicator.options;

    result = f'''\
    public class {name} : IndicatorBase<IndicatorDataPoint> {{
        IntPtr state;
        {'\n'.join(f'public Identity {output.capitalize()};') for output in outputs}
        public {name}({', '.join(map('double {}'.format, options))}) {{
            int ret = ti_{name}_stream_new(new double[]{{{', '.join(options)}}}, ref state);
            util.DispatchException(ret);
            {'\n'.join(f'{output.capitalize()} = new Identity("{output}");') for output in outputs}
        }}
        public override decimal ComputeNextValue(IndicatorDataPoint data) {{
            IntPtr[] inputs;
            IntPtr[] outputs;
            {"\n".join(f'inputs[{i}] = AllocHGlobal(sizeof(double));' for i, input in enumerate(inputs))}
            {"\n".join(f'Marshal.Copy(data.Value, 0, inputs[{i}], sizeof(double));' for i, input in enumerate(inputs))}
            {"\n".join(f'outputs[{i}] = AllocHGlobal(sizeof(double));' for i, output in enumerate(outputs))}
            int result = ti_{name}_stream_run(state, 1, inputs, outputs);
            util.DispatchException(result);
            double tmp;
            {"\n".join(f'Marshal.Copy(outputs[{i}], tmp, 0, sizeof(double)); {output.capitalize()}.Update(tmp);' for i, output in enumerate(outputs))}
            foreach (IntPtr input in inputs) {{ FreeHGlobal(input); }}
            foreach (IntPtr output in outputs) {{ FreeHGlobal(output); }}
            return {outputs[0].capitalize()}.Current.Value;
        }}
        public override bool IsReady = ti_stream_get_progress(state) > 0;
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

    indicators = '\n\n'.join(indicator(name) for name in ti.available_indicators)
    result =

    with open('tulipindicators.cs', 'w') as f:
        f.write(result)
