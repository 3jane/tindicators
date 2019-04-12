from ctypes import *
import numpy as np
from collections import namedtuple

TI_MAXINDPARAMS = 10
TI_TYPE_OVERLAY = 1      # These have roughly the same range as the input data. 
TI_TYPE_INDICATOR = 2    # Everything else (e.g. oscillators). 
TI_TYPE_MATH = 3         # These aren't so good for plotting, but are useful with formulas. 
TI_TYPE_SIMPLE =  4      # These apply a simple operator (e.g. addition, sin, sqrt). 
TI_TYPE_COMPARATIVE = 5  # These are designed to take inputs from different securities. i.e. compare stock A to stock B.

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


class _IndicatorInfo:

    type_names = {
        TI_TYPE_OVERLAY:     'overlay',
        TI_TYPE_INDICATOR:   'indicator',
        TI_TYPE_MATH:        'math',
        TI_TYPE_SIMPLE:      'simple',
        TI_TYPE_COMPARATIVE: 'comparative',
    }

    def __init__(self, lib, indicator_name):
        info = lib.ti_find_indicator(
            c_char_p(bytes(indicator_name, 'ascii'))).contents

        self.name = indicator_name
        self.type = self.type_names[info.type]
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
        inputs_lst = self.__parse_inputs(args, kwargs)
        options_lst = self.__parse_options(args, kwargs)

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

        result = np.array([np.ctypeslib.as_array(outputs[idx].contents)
                           for idx in range(len(self.info.outputs))])
        result = self.pad(result, insize) if pad_left else result

        if len(result) != 1:
            ret_t = namedtuple(
                f'{name}_{"_".join(str(opt) for opt in options_lst)}',
                self.info.outputs)
            result = self.__to_named_tuple(result, ret_t)
        else:
            result = result[0]
        return result

    def __str__(self):
        return (
            f"Name: {self.info.name},\n" 
            f"Full Name: {self.info.full_name},\n"
            f"Type: {self.info.type},\n"
            f"Inputs: {' '.join(self.info.inputs)},\n"
            f"Options: {' '.join(self.info.options)},\n"
            f"Outputs: {' '.join(self.info.outputs)}")

    def __parse_options(self, args, kwargs):
        options_lst = list(args[len(self.info.inputs):
                                len(self.info.inputs) + len(self.info.options)])
        options_lst.extend([None] * (len(self.info.options) - len(options_lst)))
        # Parse kwargs if args doesn't exsist (maybe partially)
        for idx, option_name in enumerate(self.info.options):
            if options_lst[idx] is None:
                options_lst[idx] = kwargs[option_name]
        return options_lst

    def __parse_inputs(self, args, kwargs):
        inputs_lst = list(args[:len(self.info.inputs)])
        inputs_lst.extend([None] * (len(self.info.inputs) - len(inputs_lst)))
        # Parse kwargs if args doesn't exsist (maybe partially)
        for idx, input_name in enumerate(self.info.inputs):
            if inputs_lst[idx] is None:
                inputs_lst[idx] = kwargs[input_name]
        return inputs_lst

    def __to_named_tuple(self, result, ret_t):
        result = ret_t(
            *(np.ctypeslib.as_array(result[idx])
              for idx in range(len(self.info.outputs)))
        )
        return result
        
    def pad(self, result, insize, pad=np.nan):
        outsize = result[0].size
        padsize = insize - outsize
        pad_output = np.empty((len(self.info.outputs), insize))
        for idx in range(len(result)):
            # pad_output[idx] = np.pad(result[idx], (padsize, 0),
            #                     'constant', constant_values=(np.nan,))
            pad_output[idx] = np.append([pad] * padsize, result[idx])
        return pad_output


class TulipIndicators:
    def __init__(self, shared_path='./libindicators.so'):
        self._lib = CDLL(shared_path)
        self._lib.ti_find_indicator.restype = POINTER(ti_indicator_info)

    def __getattr__(self, name):
        return _Indicator(self._lib, name)


ti = TulipIndicators()
# ti.sma(np.array([1,2,3]), period=3)
