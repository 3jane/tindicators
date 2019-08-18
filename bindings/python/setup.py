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
    name="tulipindicators",
    version=version,
    author="Ilya Pikulin",
    author_email="ilya.p@rcdb.io",
    description="Python wrapper for Tulip Indicators",
    packages=setuptools.find_packages(),
    setup_requires=SETUP_REQUIREMENTS,
    tests_require=TESTS_REQUIREMENTS,
    install_requires=INSTALL_REQUIREMENTS,
    extras_require={
        'dev': tuple(TESTS_REQUIREMENTS + DEV_REQUIREMENTS)
    },
    classifiers=["Programming Language :: Python :: 3", "Operating System :: OS Independent"],
    package_data={"tulipindicators": ["libindicators.so", "indicators.dll", "libindicators.dylib"]}
)
