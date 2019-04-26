from tulipindicators import ti
import os
import shutil

# - base class `indicator' that implements run, stream_run etc on managed arrays
# - successors that have static fields "name" etc and implement run_raw, stream_run_raw, etc
# - the only method will need to be changed: https://github.com/hcmc-project/tulipindicators-net-lean/blob/STR-172-indicators-wrapper/TulipIndicators.cs#L13

proto = '\n'.join([
    '[DllImport("indicators", EntryPoint="{fun}")]',
    '{specs} extern {ret} {as_}({args});'
])

def indicator(name):
    return f'''
public class {name} : indicator {{
    {proto.format(fun='ti_{name}'.format(name=name), as_='raw_run', ret='int', args=', '.join(['int size', 'IntPtr[] inputs', 'double[] options', 'IntPtr[] outputs']), specs='override ')}
    {proto.format(fun='ti_{name}_stream_run'.format(name=name), as_='raw_stream_run', ret='int', args=', '.join(['IntPtr stream', 'int size', 'IntPtr[] inputs', 'IntPtr[] outputs']), specs='override ')}
    {proto.format(fun='ti_{name}_stream_new'.format(name=name), as_='stream_new', ret='int', args=', '.join(['double[] options', 'ref IntPtr stream']), specs='override public')}
    {proto.format(fun='ti_{name}_stream_free'.format(name=name), as_='stream_free', ret='int', args='IntPtr stream', specs='override public')}
    {proto.format(fun='ti_{name}_start'.format(name=name), as_='start', ret='int', args='double[] options', specs='override public')}
}}
'''


if __name__ == '__main__':
    srclibpath = ti._lib._name
    dirpath = os.path.dirname(os.path.abspath(__file__))
    shutil.copy(srclibpath, dirpath)

    # functions = [
    #     proto.format(fun='stream_run', ret='int', args=', '.join(['IntPtr stream, int size, IntPtr[] inputs, IntPtr[] outputs'])),
    #     proto.format(fun='stream_free', ret='void', args=', '.join(['IntPtr stream'])),
    #     proto.format(fun='stream_get_progress', ret='int', args=', '.join(['IntPtr stream']))
    # ]

    # for name in ti.available_indicators:
    #     indicator = ti.__getattr__(name)
    #     functions.extend([
    #         proto.format(fun=f'{name}', ret='int', args=', '.join(['int size', 'IntPtr[] inputs', 'double[] options', 'IntPtr[] outputs'])),
    #         proto.format(fun=f'{name}_start', ret='int', args=', '.join(['double[] options']))
    #     ])
    #     if indicator.info.raw.stream_new:
    #         functions.extend([
    #             proto.format(fun=f'{name}_stream_new', ret='int', args=', '.join(['double[] options', 'IntPtr *stream']))
    #         ])

    # result = tulipindicators_template.format(functions='\n\n'.join(functions))

    indicators = '\n\n'.join(indicator(name) for name in ti.available_indicators)
    result = '''\
using System;
using System.Runtime.InteropServices;

class TulipIndicators {
    public abstract class indicator {
        abstract int raw_run(int size, IntPtr[] inputs, double[] options, IntPtr[] outputs);
        abstract int raw_stream_run(IntPtr stream, int size, IntPtr[] inputs, IntPtr[] outputs);
        public abstract int stream_new(double[] options, ref IntPtr stream);
        public abstract int stream_free(IntPtr stream);
        public abstract int start(double[] options);
''' f'''
        {proto.format(fun='ti_stream_get_progress', as_='stream_get_progress', ret='int', args='IntPtr stream', specs='public')}
''' '''
        int run(double[][] inputs, double[] options, double[][] outputs) {
            int size = inputs[0].Length;
            int outlen = size - start(options);

            IntPtr[] m_inputs = new IntPtr[inputs.Length];
            for (int i = 0; i < inputs.Length; ++i) {
                m_inputs[i] = Marshal.AllocHGlobal(inputs[i].Length*sizeof(double));
                Marshal.Copy(inputs[i], 0, m_inputs[i], size);
            }
            IntPtr[] m_outputs = new IntPtr[outputs.Length];
            for (int i = 0; i < outputs.Length; ++i) {
                m_outputs[i] = Marshal.AllocHGlobal(outlen * sizeof(double));
            }
            int result = raw_run(size, m_inputs, options, m_outputs);
            for (int i = 0; i < outputs.Length; ++i) {
                Marshal.Copy(m_outputs[i], outputs[i], 0, outlen);
                Marshal.FreeHGlobal(m_outputs[i]);
            }
            for (int i = 0; i < inputs.Length; ++i) {
                Marshal.FreeHGlobal(m_inputs[i]);
            }
            return result;
        }

        int stream_run(IntPtr stream, double[][] inputs, double[][] outputs) {
            int size = inputs[0].Length;
            int outlen = size + get_progress(stream);
            if (outlen < 0) { outlen = 0; }

            IntPtr[] m_inputs = new IntPtr[inputs.Length];
            for (int i = 0; i < inputs.Length; ++i) {
                m_inputs[i] = Marshal.AllocHGlobal(inputs[i].Length * sizeof(double));
                Marshal.Copy(inputs[i], 0, m_inputs[i], size);
            }
            IntPtr[] m_outputs = new IntPtr[outputs.Length];
            for (int i = 0; i < outputs.Length; ++i) {
                m_outputs[i] = Marshal.AllocHGlobal(outlen * sizeof(double));
            }
            int result = raw_stream_run(stream, size, m_inputs, m_outputs);
            for (int i = 0; i < outputs.Length; ++i) {
                Marshal.Copy(m_outputs[i], outputs[i], 0, outlen);
                Marshal.FreeHGlobal(m_outputs[i]);
            }
            for (int i = 0; i < inputs.Length; ++i) {
                Marshal.FreeHGlobal(m_inputs[i]);
            }
            return result;
        }
    } ''' f'''
    {indicators} ''' '''
}
'''

    with open('tulipindicators.cs', 'w') as f:
        f.write(result)
