# Xpress Insight Portfolio Optimization example
# Copyright (c) 2020-2024 Fair Isaac Corporation. All rights reserved.

import xpressinsight as xi
import xpress as xp
import pandas as pd
import pytest

@xi.AppConfig(name="Portfolio Optimization", version=xi.AppVersion(1, 0, 0))
class InsightApp(xi.AppBase):
    # Input entities
    MaxHighRisk: xi.types.Scalar(default=0.33, alias="Maximum investment into high-risk values")
    MaxPerShare: xi.types.Scalar(default=0.25, alias="Maximum investment per share")
    MinNorthAmerica: xi.types.Scalar(default=0.45, alias="Minimum investment into North-American values")
    ShareIds: xi.types.Index(dtype=xi.string, alias="Shares")

    # Input and result entities indexed over ShareIds
    Shares: xi.types.DataFrame(index="ShareIds", columns=[
        xi.Column("Return", dtype=xi.real, format="$ 0.00", alias="Expected Return on Investment"),
        xi.Column("HighRisk", dtype=xi.boolean, alias="High-risk value"),
        xi.Column("NorthAmerica", dtype=xi.boolean, alias="Issued in North America"),
        xi.Column("fraction", dtype=xi.real, format="0.0%", alias="Fraction used", manage=xi.Manage.RESULT)
    ])

    # Result entities
    TotalReturn: xi.types.Scalar(dtype=xi.real, alias="Total expected return on investment", manage=xi.Manage.RESULT)
    SummaryIds: xi.types.Index(dtype=xi.string, alias="Summary", manage=xi.Manage.RESULT)
    SummaryValues: xi.types.Series(index="SummaryIds", dtype=xi.real, manage=xi.Manage.RESULT)

    # Constant class attribute
    DATAFILE = "shares.csv"

    @xi.ExecModeLoad(descr="Load input data and initialize all input entities.", threads=1)
    def load(self):
        # print("Loading data.")
        self.Shares = pd.read_csv(InsightApp.DATAFILE, index_col=['ShareIds'])
        self.ShareIds = self.Shares.index
        # print("Loading finished.")

    @xi.ExecModeRun(descr="Solve problem and initialize all result entities.", threads=1)
    def run(self):
        # print('Starting optimization.')

        # Create Xpress problem and variables
        p = xp.problem("portfolio")
        self.Shares['fractionVar'] = pd.Series(xp.vars(self.ShareIds, vartype=xp.continuous, name='fractionVar'))
        p.addVariable(self.Shares.fractionVar)

        # Objective: expected total return
        objective = xp.Sum(self.Shares.Return * self.Shares.fractionVar)
        p.setObjective(objective, sense=xp.ObjSense.MAXIMIZE)

        # Limit the percentage of high-risk values
        limit_high_risk = \
            xp.Sum(self.Shares.loc[self.Shares.HighRisk, 'fractionVar']) \
            <= self.MaxHighRisk
        p.addConstraint(limit_high_risk)

        # Minimum amount of North-American values
        limit_north_america = \
            xp.Sum(self.Shares.loc[self.Shares.NorthAmerica, 'fractionVar']) \
            >= self.MinNorthAmerica
        p.addConstraint(limit_north_america)

        # Spend all the capital
        p.addConstraint(xp.Sum(self.Shares.fractionVar) == 1)

        # Upper bounds on the investment per share
        for share_id, share in self.Shares.iterrows():
            p.addConstraint(share.fractionVar <= self.MaxPerShare)

        p.setOutputEnabled(0)
        # Solve optimization problem
        p.solve()

        # Save results and key indicator values for GUI display
        self.Shares["fraction"] = pd.Series(p.getSolution(), index=self.ShareIds)
        self.TotalReturn = p.getObjVal()

        self.SummaryValues = pd.Series({
            "Expected total return": self.TotalReturn,
            "Total high risk shares": limit_high_risk.ub - p.getSlack(limit_high_risk),
            "Total North-American": limit_north_america.lb - p.getSlack(limit_north_america),
            "Largest position": self.Shares.fraction.max()
        })
        self.SummaryIds = self.SummaryValues.index
        # print('\n', self.SummaryValues, '\n', sep='')
        # print('Optimization finished.')


"""
def test_main_func():
    app = xi.create_app(InsightApp)
    app.call_exec_mode('LOAD')

    # Simulate UI behaviour
    for value in range(1,11):
        app.MaxPerShare = value/10
        app.data_connector.save_input()
        app.call_exec_mode('RUN')
        print("MaxPerShare: ", value/10, ", Return: ",app.TotalReturn)

    assert app.MaxPerShare == 1"""

if __name__ == "__main__":
    pytest.main()
