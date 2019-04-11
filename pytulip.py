from ctypes import *
import numpy as np
from collections import namedtuple

TI_MAXINDPARAMS = 10


class ti_indicator_info(Structure):
    _fields_ = [
        ("name", c_char_p),
        ("full_name", c_char_p),
        ("start", c_void_p),
        ("indicator", c_void_p),
        ("indicator_ref", c_void_p),
        ("type", c_int),
        ("inputs", c_int),
        ("options", c_int),
        ("outputs", c_int),
        ("input_names", c_char_p * TI_MAXINDPARAMS),
        ("option_names", c_char_p * TI_MAXINDPARAMS),
        ("output_names", c_char_p * TI_MAXINDPARAMS),
        ("stream_new", c_void_p),
        ("stream_run", c_void_p),
        ("stream_free", c_void_p),
    ]


class TulipIndicators:
    def __init__(self):
        self._lib = CDLL('./libindicators.so')
        self._lib.ti_find_indicator.restype = POINTER(ti_indicator_info)
    
    class caller:
        def __init__(self, lib, name):
            self._lib = lib
            self._name = name
            self._info = lib.ti_find_indicator(c_char_p(bytes(name, 'ascii')))
            self._inputs, self._outputs, self._options = [], [], []
            for idx in range(self._info.contents.inputs):
                self._inputs.append(self._info.contents.input_names[idx].decode('ascii'))
            for idx in range(self._info.contents.outputs):
                self._outputs.append(self._info.contents.output_names[idx].decode('ascii'))
            for idx in range(self._info.contents.options):
                self._options.append(self._info.contents.option_names[idx].decode('ascii'))
    
        def __call__(self, *args, **kwargs):
            # Parse args
            inputs_lst = list(args[:len(self._inputs)])
            options_lst = list(args[len(self._inputs):len(self._inputs)+len(self._options)])

            inputs_lst.extend([None] * (len(self._inputs) - len(inputs_lst)))
            options_lst.extend([None] * (len(self._options) - len(options_lst)))

            for idx, name in enumerate(self._inputs):
                if inputs_lst[idx] is None:
                    inputs_lst[idx] = kwargs[name]
            for idx, name in enumerate(self._options):
                if options_lst[idx] is None:
                    options_lst[idx] = kwargs[name]

            # Do the ctypes-related machinery
            options = (c_double * len(self._options))()
            for idx, option in enumerate(options_lst):
                options[idx] = option

            name = self._name
            insize = inputs_lst[0].size
            start_fun = self._lib.__getattr__(f'ti_{name}_start')
            start_fun.restype = c_int
            outsize = insize - start_fun(options)

            inputs = (POINTER(c_double * insize) * len(self._inputs))()
            for idx, input in enumerate(inputs_lst):
                inp = (c_double * insize)(*inputs_lst[idx])
                inputs[idx] = pointer(inp)

            outputs = (POINTER(c_double * outsize) * len(self._outputs))()
            for idx in range(len(self._outputs)):
                out = (c_double * outsize)()
                outputs[idx] = pointer(out)

            fun = self._lib.__getattr__(f'ti_{name}')
            fun.restype = c_int
            errcode = fun(insize, inputs, options, outputs)

            if errcode != 0:
                raise Exception(f'{name} returned {errcode}')

            # Maybe DataFrame?
            ret_t = namedtuple(f'{name}_{"_".join(str(opt) for opt in options_lst)}', self._outputs)
            return ret_t(*(np.ctypeslib.as_array(outputs[idx].contents) for idx in range(len(self._outputs))))

    def __getattr__(self, name):
        return self.caller(self._lib, name)
