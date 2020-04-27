import os
import itertools
import platform
from os import path
from pathlib import Path
from setuptools import Extension
from distutils.core import setup
from Cython.Build import cythonize

if platform.system() == 'Darwin':
    os.environ['LDFLAGS'] = '-framework Security'

HERE = path.dirname(path.abspath(__file__))
CORE_ROOT = path.abspath(path.join(HERE, path.pardir, 'WalletKitCore'))
CORE_SRC_FILES = list(map(str, itertools.chain(
    Path(path.join(CORE_ROOT, 'src')).rglob('*.c'),
    Path(path.join(CORE_ROOT, 'vendor', 'ed25519')).rglob('*.c')
)))
CYTHON_INCLUDE_DIRS = [path.join(HERE, 'walletkit', 'native')]
INCLUDE_DIRS = [
    path.join(CORE_ROOT, 'src'),
    path.join(CORE_ROOT, 'include'),
    path.join(CORE_ROOT, 'vendor'),
    path.join(CORE_ROOT, 'vendor', 'secp256k1'),
]
NATIVE_MODULES = ['hasher']
EXTENSIONS = []
LIBRARIES = ['WalletKitCore']
LIBRARY_DIRS = [path.join(HERE, 'walletkit', 'native')]

for wkmod in NATIVE_MODULES:
    src = [path.join(HERE, 'walletkit', 'native', f'{wkmod}.pyx')] + CORE_SRC_FILES
    EXTENSIONS.append(Extension(
        wkmod, src,
        include_dirs=INCLUDE_DIRS,
        libraries=["resolv"],
        extra_compile_args=[
            "-Wall",
            "-Wconversion",
            "-Wsign-conversion",
            "-Wparentheses",
            "-Wswitch",
            "-Wno-implicit-int-conversion",
            "-Wno-missing-braces",
        ]
    ))

setup(
    name='walletkit',
    version='0.1.0',
    author="Samuel Sutch",
    author_email="sam@blockset.com",
    description="A wrapper around the WalletKit library",
    license="MIT",
    url="https://github.com/blockset-corp/walletkit",
    packages=['walletkit'],
    ext_package='walletkit.native',
    ext_modules=cythonize(
        EXTENSIONS,
        include_path=CYTHON_INCLUDE_DIRS,
        language_level=3
    ),
    zip_safe=False
)
