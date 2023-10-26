# setup.py
# 
from setuptools import setup
from Cython.Build import cythonize
from setuptools.extension import Extension

sourcefiles = ['tinylemmatizer.c', 'cstlempy.pyx']

extensions = [Extension("cstlempy", sourcefiles, extra_compile_args=["-I.", "-D_CRT_SECURE_NO_WARNINGS","-DPYTHONINTERFACE"])]

setup(name="CSTtinylemmatizer",
    ext_modules = cythonize(extensions, emit_linenums=True, compiler_directives={'language_level': 3})
)
