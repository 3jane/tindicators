from tulipindicators import ti
import os
import shutil

if __name__ == '__main__':
    srclibpath = ti._lib._name
    dirpath = os.path.dirname(os.path.abspath(__file__))
    shutil.copy(srclibpath, dirpath)

    proto = '\n'.join([
        '[DllImport("indicators", EntryPoint="ti_{fun}")]',
        'unsafe public static extern {ret} {fun}({args});'
    ])

    functions = [
        proto.format(fun='stream_run', ret='int', args=', '.join(['void *stream, int size, double*[] inputs, double*[] outputs'])),
        proto.format(fun='stream_free', ret='void', args=', '.join(['void *stream'])),
        proto.format(fun='stream_get_progress', ret='int', args=', '.join(['void *stream']))
    ]

    for name in ti.available_indicators:
        indicator = ti.__getattr__(name)
        functions.extend([
            proto.format(fun=f'{name}', ret='int', args=', '.join(['int size', 'double*[] inputs', 'double[] options', 'double*[] outputs'])),
            proto.format(fun=f'{name}_start', ret='int', args=', '.join(['double[] options']))
        ])
        if indicator.info.raw.stream_new:
            functions.extend([
                proto.format(fun=f'{name}_stream_new', ret='int', args=', '.join(['double[] options', 'void **stream']))
            ])

    result = '\n'.join([
        'using System;',
        'using System.Runtime.InteropServices;',
        '',
        'public class TulipIndicators {{',
        '',
        '{functions}',
        '',
        '}}',
        ''
    ]).format(functions='\n\n'.join(functions))

    with open('tulipindicators.cs', 'w') as f:
        f.write(result)
