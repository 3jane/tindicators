import setuptools

import sys
sys.path.insert(0, '../..')
from version import version

setuptools.setup(
    name="tulipindicators",
    version=version,
    author="Ilya Pikulin",
    author_email="ilya.p@rcdb.io",
    description="Python wrapper for Tulip Indicators",
    packages=setuptools.find_packages(),
    install_requires=["numpy"],
    classifiers=["Programming Language :: Python :: 3", "Operating System :: OS Independent"],
    package_data={"tulipindicators": ["libindicators.so", "indicators.dll"]}
)
