# *********************************************************
#   Mosel Example Problems
#   ======================
#
#   file folioqp_py.py
#   ``````````````````
#   Python function definitions for Mosel parent model
#   folioqp_py.mos.
#
#   (c) 2019 Fair Isaac Corporation
#       author: J. Müller
# *********************************************************
import platform
import numpy as np
import pandas as pd


def covariance_of_series(series):
    """ Compute covariance matrix of two-dimensional series. """

    covariance_data_frame = series.unstack(level = 0).cov()
    print(covariance_data_frame)

    return covariance_data_frame.stack()


if __name__ == "__main__":
    print("Python library version: ", platform.python_version())
    print("NumPy library version:  ", np.__version__)
    print("pandas library version: ", pd.__version__)
