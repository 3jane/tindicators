# This file is part of tindicators, licensed under GNU LGPL v3.
# Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019
# Author: Anatoly <elengar17@gmail.com>, 2019


import setuptools

import sys
sys.path.insert(0, '../..')
from version import version


INSTALL_REQUIREMENTS = [
    "numpy"
]

SETUP_REQUIREMENTS = [
    "pytest-runner"
]

TESTS_REQUIREMENTS = [
    "tox",
    "tox-wheel",
    "pytest",
    "pandas"
]

DEV_REQUIREMENTS = [

]


setuptools.setup(
    name="tindicators",
    version=version,
    author="Ilya Pikulin",
    author_email="ilya.pikulin@gmail.com",
    description="Python wrapper for tindicators",
    packages=setuptools.find_packages(),
    setup_requires=SETUP_REQUIREMENTS,
    tests_require=TESTS_REQUIREMENTS,
    install_requires=INSTALL_REQUIREMENTS,
    extras_require={
        'dev': tuple(TESTS_REQUIREMENTS + DEV_REQUIREMENTS)
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: Microsoft :: Windows',
        'Operating System :: POSIX :: Linux',
    ],
    package_data={"tindicators": ["libindicators.so", "indicators.dll", "libindicators.dylib"]}
)
