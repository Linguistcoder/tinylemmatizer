#!/bin/bash
rm cstlempy.c*
rm -r build
python setup.py build_ext --inplace
python launch.py
