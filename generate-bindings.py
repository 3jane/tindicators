from tulipindicators import ti
import os
import shutil

tmpl_toplevel = '''\
using System;
using System.Runtime.InteropServices;
using QuantConnect.Data.Market;
using QuantConnect.Indicators;

namespace Rcdb.TulipIndicators {
    $util
    namespace Streaming {
        $streaming
    }
    namespace Default {
        $default
    }
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
    if not indicator.raw.stream_new:
        return ''

    input_type = ['TradeBar', 'IndicatorDataPoint'][real]

    result = f'''
    public class {name} : IndicatorBase<{input_type}> {{
        IntPtr state;
        {f'{n}'.join(f'public Identity {output.upper()};' for output in outputs)}
        public {name}({', '.join(map('double {}'.format, options))}) : base("{name}") {{
            int ret = ti_{name}_stream_new(new double[]{{{', '.join(options)}}}, ref state);
            util.DispatchError(ret);
            {f'{n}'.join(f'{output.upper()} = new Identity("{output}");' for output in outputs)}
        }}
        protected override decimal ComputeNextValue({input_type} data) {{
            IntPtr[] inputs = new IntPtr[{len(inputs)}];
            IntPtr[] outputs = new IntPtr[{len(outputs)}];
            double[] tmp = new double[1];
            {f"{n}".join(f'inputs[{i}] = Marshal.AllocHGlobal(1);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'tmp[{i}] = (double)data.{input.capitalize() if not real else "Value"};' for i, input in enumerate(inputs))}
            {f"{n}".join(f'Marshal.Copy(tmp, 0, inputs[{i}], 1);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'outputs[{i}] = Marshal.AllocHGlobal(1);' for i, output in enumerate(outputs))}
            int result = ti_{name}_stream_run(state, 1, inputs, outputs);
            util.DispatchError(result);
            {f"{n}".join(f'Marshal.Copy(outputs[{i}], tmp, 0, 1); {output.upper()}.Update(data.Time, (decimal)tmp[0]);' for i, output in enumerate(outputs))}
            foreach (IntPtr input in inputs) {{ Marshal.FreeHGlobal(input); }}
            foreach (IntPtr output in outputs) {{ Marshal.FreeHGlobal(output); }}
            return (decimal){outputs[0].upper()}.Current.Value;
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

def default(indicator):
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
    public class {name} : WindowIndicator<{input_type}> {{
        double[] options;
        int start;
        bool ready;
        {f'{n}'.join(f'public Identity {output.upper()};' for output in outputs)}
        public {name}({', '.join(map('double {}'.format, options))}, int windowsize=0)
            : base("{name}", Math.Max(windowsize, ti_{name}_start(options) + 1)) {{
            options = new double[]{{{', '.join(options)}}};
            start = ti_{name}_start(options);
            ready = false;
            {f'{n}'.join(f'{output.upper()} = new Identity("{output}");' for output in outputs)}
        }}
        protected override decimal ComputeNextValue(RollingWindow<{input_type}> window, {input_type} data) {{
            ready = window.Size >= start;
            if (!ready) {{ return 0; }}
            IntPtr[] inputs = new IntPtr[{len(inputs)}];
            IntPtr[] outputs = new IntPtr[{len(outputs)}];
            double[] tmp = new double[window.Size];
            {f"{n}".join(f'inputs[{i}] = Marshal.AllocHGlobal(window.Size);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'{{ int i; foreach ({input_type} value of window) {{ tmp[i] = (double)data.{input.capitalize() if not real else "Value"}; }}; i += 1; }}' for i, input in enumerate(inputs))}
            {f"{n}".join(f'Marshal.Copy(tmp, 0, inputs[{i}], window.Size);' for i, input in enumerate(inputs))}
            {f"{n}".join(f'outputs[{i}] = Marshal.AllocHGlobal(window.Size - start);' for i, output in enumerate(outputs))}
            int result = ti_{name}(window.Size, inputs, options, outputs);
            util.DispatchError(result);
            {f"{n}".join(f'Marshal.Copy(outputs[{i}], tmp, 0, window.Size - start); {output.upper()}.Update(data.Time, (decimal)tmp[window.Size - start - 1]);' for i, output in enumerate(outputs))}
            foreach (IntPtr input in inputs) {{ Marshal.FreeHGlobal(input); }}
            foreach (IntPtr output in outputs) {{ Marshal.FreeHGlobal(output); }}
            return (decimal){outputs[0].upper()}.Current.Value;
        }}
        public override bool IsReady {{
            get {{ return ready; }}
        }}
        {dllimport.format(fun=f'ti_{name}_start', ret='int', args='double[] options, ref IntPtr state')}
        {dllimport.format(fun=f'ti_{name}', ret='void', args='IntPtr state')}
    }}
    '''
    return result

if __name__ == '__main__':
    srclibpath = ti._lib._name
    dirpath = os.path.dirname(os.path.abspath(__file__))
    shutil.copy(srclibpath, dirpath)

    result = tmpl_toplevel \
        .replace('$util', util) \
        .replace('$streaming', '\n'.join(streaming(ti.__getattr__(name).info) for name in ti.available_indicators)) \
        .replace('$default', '\n'.join(streaming(ti.__getattr__(name).info) for name in ti.available_indicators))

    with open('tulipindicators.cs', 'w') as f:
        f.write(result)
