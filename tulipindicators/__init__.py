from ctypes import *
import numpy as np
from collections import namedtuple
import os
from datetime import datetime

TI_MAXINDPARAMS = 10
TI_TYPE_OVERLAY = 1      # These have roughly the same range as the input data.
TI_TYPE_INDICATOR = 2    # Everything else (e.g. oscillators).
TI_TYPE_MATH = 3         # These aren't so good for plotting, but are useful with formulas.
TI_TYPE_SIMPLE = 4       # These apply a simple operator (e.g. addition, sin, sqrt).
TI_TYPE_COMPARATIVE = 5  # These are designed to take inputs from different securities. i.e. compare stock A to stock B.

class InvalidOption(Exception): pass
class OutOfMemory(Exception): pass
class NoSuchIndicator(Exception): pass

ret2exc = {
    1: InvalidOption,
    2: OutOfMemory,
}

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
        try: info = lib.ti_find_indicator(c_char_p(bytes(indicator_name, 'ascii'))).contents
        except: raise NoSuchIndicator(indicator_name)

        self.name = indicator_name
        self.type = self.type_names[info.type]
        self.full_name = info.full_name.decode('ascii')

        self.inputs = tuple(info.input_names[i].decode('ascii').replace(' ', '_') for i in range(info.inputs))
        self.outputs = tuple(info.output_names[i].decode('ascii').replace(' ', '_') for i in range(info.outputs))
        self.options = tuple(info.option_names[i].decode('ascii').replace(' ', '_') for i in range(info.options))

class _Indicator:
    def __init__(self, lib, name):
        self.info = _IndicatorInfo(lib, name)
        self.__start = lib.__getattr__(f'ti_{name}_start')
        self.__run = lib.__getattr__(f'ti_{name}')
        self.__start.restype = c_int
        self.__run.restype = c_int

    def __repr__(self):
        return '\n'.join((
            f"Name:     \t{self.info.name}",
            f"Full Name:\t{self.info.full_name}",
            f"Type:     \t{self.info.type}",
            f"Inputs:   \t{' '.join(self.info.inputs)}",
            f"Options:  \t{' '.join(self.info.options)}",
            f"Outputs:  \t{' '.join(self.info.outputs)}"))

    def __call__(self, *args, pad=True, **kwargs):
        inputs_lst = self.__parse(args, kwargs, 0, self.info.inputs)
        options_lst = self.__parse(args, kwargs, len(self.info.inputs), self.info.options)

        options = (c_double * len(self.info.options))() # double options[]
        for idx, option in enumerate(options_lst):
            options[idx] = option

        insize = len(inputs_lst[0])
        start_amount = self.__start(options)
        outsize = insize - start_amount

        inputs = (POINTER(c_double * insize) * len(self.info.inputs))() # double *inputs[]
        for idx in range(len(self.info.inputs)):
            inp = (c_double * insize)(*inputs_lst[idx]) # ensures that the array is not temporary
            inputs[idx] = pointer(inp)

        outputs = (POINTER(c_double * outsize) * len(self.info.outputs))() # double *outputs[]
        for idx in range(len(self.info.outputs)):
            out = (c_double * outsize)() # allocates memory
            outputs[idx] = pointer(out)

        elaborated_name = self.__construct_elaborated_name(self.info.name, options_lst)

        errcode = self.__run(insize, inputs, options, outputs)
        if errcode != 0:
            raise ret2exc[errcode](f'{elaborated_name}')

        ret_t = namedtuple(elaborated_name, self.info.outputs)
        outputs_lst = [np.ctypeslib.as_array(outputs[idx].contents) for idx in range(len(self.info.outputs))]

        if pad:
            for idx in range(len(outputs_lst)):
                outputs_lst[idx] = self.pad(outputs_lst[idx], start_amount)

        result = ret_t(*outputs_lst)
        return result if len(self.info.outputs) > 1 else result[0]

    def __construct_elaborated_name(self, name, options_lst):
        return name + '_'.join(str(opt).replace('.', 'p').replace('-', 'm') for opt in [''] + options_lst)

    def __parse(self, args, kwargs, offset, names):
        size = len(names)
        result = list(args[offset:offset + size])
        for idx in range(len(result), size):
            result.append(kwargs[names[idx]])
        return result

    def pad(self, output, padsize, pad=np.nan):
        return np.pad(output, (padsize, 0), 'constant', constant_values=(np.nan,))

class TulipIndicators:
    def __init__(self, sharedlib_path=None):
        dir_path = os.path.dirname(os.path.abspath(__file__))
        if sharedlib_path is None:
            if os.name == 'nt':
                sharedlib_path = os.path.join(dir_path, r'indicators.dll')
            elif os.name == 'posix':
                sharedlib_path = os.path.join(dir_path, 'libindicators.so')
        self._lib = CDLL(sharedlib_path)
        self._lib.ti_find_indicator.restype = POINTER(ti_indicator_info)
        self._lib.ti_build.restype = c_long

        self._indicator_count = self._lib.ti_indicator_count()
        self._build = self._lib.ti_build()
        ti_indicators = (ti_indicator_info * self._indicator_count).in_dll(self._lib, "ti_indicators")

        self.available_indicators = [ti_indicators[idx].name.decode('ascii') for idx in range(self._indicator_count)]

    def __getattr__(self, name):
        return _Indicator(self._lib, name)

    def __repr__(self):
        return '\n'.join([
            f'Tulip Indicators, built {datetime.utcfromtimestamp(self._build).strftime("%d.%m.%Y")}',
            ' '.join([f'{self._indicator_count} indicators are available:'] + self.available_indicators)
        ])

ti = TulipIndicators()