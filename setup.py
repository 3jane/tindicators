import setuptools

setuptools.setup(
    name="tulipindicators",
    version="0.1",
    author="Ilya Pikulin",
    author_email="ilya.p@rcdb.io",
    description="Python wrapper for tulipindicators",
    packages=setuptools.find_packages(),
    classifiers=["Programming Language :: Python :: 3", "Operating System :: OS Independent"],
    package_data={"": ["libindicators.so", "libindicators.dll"]}
)