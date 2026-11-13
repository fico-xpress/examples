# Example file to set up parametrized unit tests for Xpress Insight - Portfolio Optimization example
# Copyright (c) 2020-2024 Fair Isaac Corporation. All rights reserved.

import os
import sys

import pytest
import xpressinsight as xi

sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'python_source'))
from application import InsightApp


@pytest.mark.parametrize(
    ('input_value', 'expected_output'),
    (
            (0.1, 0.0),
            (0.2, 13.42),
            (0.3, 14.24),
    )
)
def test_app(input_value, expected_output, tmp_path):
    """ Parametrized testing """

    app = xi.create_app(InsightApp, app_work_dir=str(tmp_path))
    app.call_exec_mode('LOAD')

    app.MaxPerShare = input_value
    app.data_connector.save_input()
    app.call_exec_mode('RUN')

    assert app.TotalReturn == pytest.approx(expected_output)


if __name__ == "__main__":
    pytest.main()
