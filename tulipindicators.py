from ctypes import *
import numpy as np
from collections import namedtuple

TI_MAXINDPARAMS = 10

import code


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


# _type_names = {
#     ti.TI_TYPE_OVERLAY:     b'overlay',
#     ti.TI_TYPE_INDICATOR:   b'indicator',
#     ti.TI_TYPE_MATH:        b'math',
#     ti.TI_TYPE_SIMPLE:      b'simple',
#     ti.TI_TYPE_COMPARATIVE: b'comparative',
# }


class _IndicatorInfo:
    def __init__(self, lib, indicator_name):
        info = lib.ti_find_indicator(
            c_char_p(bytes(indicator_name, 'ascii'))).contents

        self.name = indicator_name
        # self.type = info.type.decode()
        self.full_name = info.full_name.decode('ascii')

        self.inputs = tuple(info.input_names[i].decode('ascii')
                       for i in range(info.inputs))
        self.outputs = tuple(info.output_names[i].decode('ascii')
                        for i in range(info.outputs))
        self.options = tuple(info.option_names[i].decode('ascii')
                        for i in range(info.options))


class _Indicator:
    def __init__(self, lib, name):
        self._lib = lib
        self._name = name
        self.info = _IndicatorInfo(lib, name)


    def __call__(self, *args, pad_left=True, **kwargs):
        # Parse args
        inputs_lst = list(args[:len(self.info.inputs)])
        options_lst = list(args[len(self.info.inputs):
                                len(self.info.inputs) + len(self.info.options)])

        inputs_lst.extend([None] * (len(self.info.inputs) - len(inputs_lst)))
        options_lst.extend([None] * (len(self.info.options) - len(options_lst)))

        for idx, input_name in enumerate(self.info.inputs):
            if inputs_lst[idx] is None:
                inputs_lst[idx] = kwargs[input_name]
        for idx, option_name in enumerate(self.info.options):
            if options_lst[idx] is None:
                options_lst[idx] = kwargs[option_name]

        # Do the ctypes-related machinery
        options = (c_double * len(self.info.options))()
        for idx, option in enumerate(options_lst):
            options[idx] = option

        name = self.info.name
        insize = inputs_lst[0].size
        start_fun = self._lib.__getattr__(f'ti_{name}_start')
        start_fun.restype = c_int
        outsize = insize - start_fun(options)

        inputs = (POINTER(c_double * insize) * len(self.info.inputs))()
        for idx in range(len(self.info.inputs)):
            inp = (c_double * insize)(*inputs_lst[idx])
            inputs[idx] = pointer(inp)

        outputs = (POINTER(c_double * outsize) * len(self.info.outputs))()
        for idx in range(len(self.info.outputs)):
            out = (c_double * outsize)()
            outputs[idx] = pointer(out)

        fun = self._lib.__getattr__(f'ti_{name}')
        fun.restype = c_int

        errcode = fun(insize, inputs, options, outputs)
        assert errcode == 0, f'{name} returned {errcode}'

        ret_t = namedtuple(
            f'{name}_{"_".join(str(opt) for opt in options_lst)}',
            self.info.outputs)
        result = ret_t(
            *(np.ctypeslib.as_array(outputs[idx].contents)
              for idx in range(len(self.info.outputs)))
        )

        return self.pad(result, insize) if pad_left else result

    def pad(self, result, insize, pad=np.nan):
        outsize = result[0].size
        padsize = insize - outsize
        pad_output = np.empty((len(self._outputs), insize))
        for idx in range(len(result)):
            pad_output[idx] = np.append(
                [pad] * padsize, result[idx])
        return pad_output

    # def __str__(self):
    #     return f"Type: {self.info.}, Full Name: Vector Square Root; Inputs: ['real'] Options: [] Outputs: ['sqrt']"


class TulipIndicators:
    def __init__(self, shared_path='./libindicators.so'):
        self._lib = CDLL(shared_path)
        self._lib.ti_find_indicator.restype = POINTER(ti_indicator_info)

    def __getattr__(self, name):
        return _Indicator(self._lib, name)


ti = TulipIndicators()


# sma = ti.sma(np.array([1, 2, 3]), 2)
# print(sma)
