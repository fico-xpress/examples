#!/usr/bin/env python
# -*- coding: utf-8 -*-
# ******************************************************
#   Mosel Python Example Problems
#   =============================
#
#   file io_example.py
#   ``````````````````
#   Python I/O driver example for the initialization of
#   data to and from Python.
#
#   (c) 2018 Fair Isaac Corporation
#       author: J.Müller
# *******************************************************

print("Values initialized to Python:")
print("  MyRange =", MyRange)
print("  A =", A)
print("Modifying data in Python...")
A = {i: 2 * i for i in MyRange if i % 2 == 0}
