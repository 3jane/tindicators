from ctypes import *
import numpy as np
from collections import namedtuple

TI_MAXINDPARAMS = 10
TI_TYPE_OVERLAY = 1      # These have roughly the same range as the input data. 
TI_TYPE_INDICATOR = 2    # Everything else (e.g. oscillators). 
TI_TYPE_MATH = 3         # These aren't so good for plotting, but are useful with formulas. 
TI_TYPE_SIMPLE = 4      # These apply a simple operator (e.g. addition, sin, sqrt). 
TI_TYPE_COMPARATIVE = 5  # These are designed to take inputs from different securities. i.e. compare stock A to stock B.


class IndicatorExecutionError(Exception):
    pass 


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
        self.info = _IndicatorInfo(lib, name)
        self.__start = lib.__getattr__(f'ti_{name}_start')
        self.__run = lib.__getattr__(f'ti_{name}')
        self.__start.restype = c_int
        self.__run.restype = c_int

    def __str__(self):
        return (
            f"Name: {self.info.name},\n" 
            f"Full Name: {self.info.full_name},\n"
            f"Type: {self.info.type},\n"
            f"Inputs: {' '.join(self.info.inputs)},\n"
            f"Options: {' '.join(self.info.options)},\n"
            f"Outputs: {' '.join(self.info.outputs)}")

    def __call__(self, *args, pad=True, **kwargs):
        inputs_lst = self.__parse_inputs(args, kwargs)
        options_lst = self.__parse_options(args, kwargs)

        options = self.__get_coptions(options_lst)
        inputs = self.__get_cinputs(inputs_lst)

        insize = inputs_lst[0].size
        outsize = insize - self.__start(options)
        padsize = insize - outsize if pad else 0
        
        outputs = self.__get_coutputs(outsize)

        errcode = self.__run(insize, inputs, options, outputs)
        if errcode != 0:
            IndicatorExecutionError(
                f'Indicator return error code {errcode}, but 0 expected.')

        if len(self.info.outputs) == 1:
            result = np.ctypeslib.as_array(outputs[0].contents)
            return self.pad(result, padsize)
        return self.__to_named_tuple(outputs, options_lst, padsize)

    def __parse_options(self, args, kwargs):
        options_lst = list(args[len(self.info.inputs):
                                len(self.info.inputs) + len(self.info.options)])
        options_lst.extend([None] * (len(self.info.options) - len(options_lst)))
        for idx, option_name in enumerate(self.info.options):
            if options_lst[idx] is None:
                options_lst[idx] = kwargs[option_name]
        return options_lst

    def __parse_inputs(self, args, kwargs):
        inputs_lst = list(args[:len(self.info.inputs)])
        inputs_lst.extend([None] * (len(self.info.inputs) - len(inputs_lst)))
        for idx, input_name in enumerate(self.info.inputs):
            if inputs_lst[idx] is None:
                inputs_lst[idx] = kwargs[input_name]
        return inputs_lst

    def __get_coptions(self, options_lst):
        options = (c_double * len(self.info.options))()
        for idx, option in enumerate(options_lst):
            options[idx] = option
        return options

    def __get_cinputs(self, inputs_lst):
        insize = inputs_lst[0].size
        inputs = (POINTER(c_double * insize) * len(self.info.inputs))()
        for idx in range(len(self.info.inputs)):
            inp = (c_double * insize)(*inputs_lst[idx])
            inputs[idx] = pointer(inp)
        return inputs

    def __get_coutputs(self, outsize):
        outputs = (POINTER(c_double * outsize) * len(self.info.outputs))()
        for idx in range(len(self.info.outputs)):
            out = (c_double * outsize)()
            outputs[idx] = pointer(out)
        return outputs

    def __to_named_tuple(self, outputs, options_lst, padsize):
        ret_t = namedtuple(
            self.info.name + '_'.join(str(opt) for opt in [''] + options_lst), 
            self.info.outputs)
        result = ret_t(
            *[self.pad(np.ctypeslib.as_array(outputs[idx].contents), padsize)
              for idx in range(len(self.info.outputs))]
        )
        return result

    def pad(self, output, padsize, pad=np.nan):
        return np.pad(output, (padsize, 0),
                      'constant', constant_values=(np.nan,))


class TulipIndicators:
    def __init__(self, sharedlib_path='./libindicators.so'):
        self._lib = CDLL(sharedlib_path)
        self._lib.ti_find_indicator.restype = POINTER(ti_indicator_info)

    def __getattr__(self, name):
        return _Indicator(self._lib, name)


ti = TulipIndicators()
