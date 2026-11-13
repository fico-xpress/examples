# Xpress Insight Portfolio Optimization example - Lightweight execution
# Copyright (c) 2020-2024 Fair Isaac Corporation. All rights reserved.

import xpressinsight as xi
import xpress as xp
import pandas as pd
import sys

@xi.AppConfig(name="Portfolio Optimization - Lightweight example", version=xi.AppVersion(1, 0, 0), partial_populate=True)
class InsightApp(xi.AppBase):
    # Input entities
    MaxHighRisk: xi.types.Scalar(default=0.33, alias="Maximum investment into high-risk values")
    MaxPerShare: xi.types.Scalar(default=0.25, alias="Maximum investment per share")
    MinNorthAmerica: xi.types.Scalar(default=0.45, alias="Minimum investment into North-American values")
    ShareIds: xi.types.Index(dtype=xi.string, alias="Shares")

    # Input and result entities indexed over ShareIds
    Shares: xi.types.DataFrame(index="ShareIds", columns=[
        xi.types.Column("Return", dtype=xi.real, format="$ 0.00", alias="Expected Return on Investment"),
        xi.types.Column("HighRisk", dtype=xi.boolean, alias="High-risk value"),
        xi.types.Column("NorthAmerica", dtype=xi.boolean, alias="Issued in North America"),
        xi.types.Column("fraction", dtype=xi.real, format="0.0%", alias="Fraction used", manage=xi.Manage.RESULT)
    ])

    # Result entities
    TotalReturn: xi.types.Scalar(dtype=xi.real, alias="Total expected return on investment", manage=xi.Manage.RESULT)
    SummaryIds: xi.types.Index(dtype=xi.string, alias="Summary", manage=xi.Manage.RESULT)
    SummaryValues: xi.types.Series(index="SummaryIds", dtype=xi.real, manage=xi.Manage.RESULT)

    # New result entities
    ReturnHighRisk: xi.types.Scalar(dtype=xi.real, alias="Average return of high-risk shares", manage=xi.Manage.RESULT)
    ReturnLowRisk: xi.types.Scalar(dtype=xi.real, alias="Average return of low-risk shares", manage=xi.Manage.RESULT)

    # Constant class attribute
    DATAFILE = "shares.csv"

    @xi.ExecModeLoad(descr="Load input data and initialize all input entities.")
    def load(self):
        print("Loading data.")
        self.Shares = pd.read_csv(InsightApp.DATAFILE, index_col=['ShareIds'])
        self.ShareIds = self.Shares.index
        print("Loading finished.")

    @xi.ExecMode(name="CUSTOM_RUN", descr="Custom run execution mode.")
    def my_custom_run_mode(self):
        print('Executing custom run mode.')
        self.insight.populate(["Shares_Return","Shares_HighRisk"])
        self.ReturnHighRisk = self.Shares.loc[self.Shares['HighRisk'] == True, 'Return'].mean()
        self.ReturnLowRisk = self.Shares.loc[self.Shares['HighRisk'] == False, 'Return'].mean()
        self.insight.capture(["ReturnHighRisk","ReturnLowRisk"])
        print('Custom run mode finished.')

    @xi.ExecModeRun(descr="Solve problem and initialize all result entities.")
    def run(self):
        print('Starting optimization.')

        self.insight.populate()

        # Create Xpress problem and variables
        p = xp.problem("portfolio")
        self.Shares['fractionVar'] = pd.Series(p.addVariables(self.ShareIds, vartype=xp.continuous, name='fractionVar'))

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
        p.addConstraint(share.fractionVar <= self.MaxPerShare for share_id, share in self.Shares.iterrows())

        # Solve optimization problem
        p.optimize()

        # Save results and key indicator values for GUI display
        self.Shares["fraction"] = pd.Series(p.getSolution(), index=self.ShareIds)
        self.TotalReturn = p.attributes.objval

        self.SummaryValues = pd.Series({
            "Expected total return": self.TotalReturn,
            "Total high risk shares": limit_high_risk.ub - p.getSlacks(limit_high_risk),
            "Total North-American": limit_north_america.lb - p.getSlacks(limit_north_america),
            "Largest position": self.Shares.fraction.max()
        })
        self.SummaryIds = self.SummaryValues.index
        print('\n', self.SummaryValues, '\n', sep='')
        print('Optimization finished.')

        self.insight.capture(entity_filter=lambda e: e.name not in ["ReturnHighRisk","ReturnLowRisk"])

if __name__ == "__main__":
    app = xi.create_app(InsightApp)
    sys.exit(app.call_exec_modes(["LOAD", "CUSTOM_RUN", "RUN"]))
