import setuptools

setuptools.setup(
    name="tulipindicators",
    version="0.1.1",
    author="Ilya Pikulin",
    author_email="ilya.p@rcdb.io",
    description="Python wrapper for tulipindicators",
    packages=setuptools.find_packages(),
    install_requires=["numpy"],
    classifiers=["Programming Language :: Python :: 3", "Operating System :: OS Independent"],
    package_data={"tulipindicators": ["libindicators.so", "indicators.dll"]}
)