#!/usr/bin/env python
# -*- coding: utf-8 -*-
# *********************************************************
#   Mosel Python Example Problems
#   =============================
#
#   file invert_matrix.py
#   `````````````````````
#   Invert a Mosel matrix in Python using pandas and NumPy.
#
#   (c) 2019 Fair Isaac Corporation
#       author: J. Müller
# *********************************************************
import platform
import numpy as np
import pandas as pd
from numpy.linalg import inv


def invert_matrix(series):
    """ Compute inverse matrix of a Series with a two-dimensional index.

    Parameters
    ----------
    series : pandas.Series
        Series with a two-dimensional zero based integer index.

    Returns
    -------
    NumPy.ndarray
        Inverse matrix as two-dimensional NumPy ndarray.
    """
    # Get pivot table of MultiIndex Series as DataFrame.
    df = series.unstack()

    # Compute inverse matrix.
    np_a_inverse = inv(df)
    print(np_a_inverse)

    # Return result as NumPy ndarray.
    return np_a_inverse


if __name__ == "__main__":
    print("Python library version: ", platform.python_version())
    print("NumPy library version:  ", np.__version__)
    print("pandas library version: ", pd.__version__)
