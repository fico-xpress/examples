# Xpress Insight Portfolio Optimization - Getting started example
# Copyright (c) 2025-2026 Fair Isaac Corporation. All rights reserved.

import xpressinsight as xi
import xpress as xp
import pandas as pd
import sys

@xi.AppConfig(name="Portfolio Optimization", version=xi.AppVersion(1, 0, 0))
class InsightApp(xi.AppBase):
    # Input entities
    MaxHighRisk: xi.types.Param(default=1/3)
    MaxPerShare: xi.types.Param(default=0.3)
    MinNorthAmerica: xi.types.Param(default=0.5)
    ShareIds: xi.types.Index(dtype=xi.string, alias="Shares")

    # Input and result entities indexed over ShareIds
    Shares: xi.types.DataFrame(index="ShareIds", columns=[
        xi.types.Column("Return", dtype=xi.real, alias="Expected Return on Investment"),
        xi.types.Column("HighRisk", dtype=xi.boolean, alias="High-risk value"),
        xi.types.Column("NorthAmerica", dtype=xi.boolean, alias="Issued in North America"),
        xi.types.Column("fraction", dtype=xi.real, alias="Fraction used", manage=xi.Manage.RESULT)
    ])

    # Result entities
    TotalReturn: xi.types.Scalar(dtype=xi.real, alias="Total expected return on investment", manage=xi.Manage.RESULT)

    CtrSolIds: xi.types.Index(dtype=xi.string, alias="Constraints", manage=xi.Manage.RESULT)
    CtrSol: xi.types.DataFrame(index="CtrSolIds", columns=[
        xi.types.Column("Activity", dtype=xi.real, alias="Activity", manage=xi.Manage.RESULT),
        xi.types.Column("LowerLimit", dtype=xi.real, alias="Lower limit", manage=xi.Manage.RESULT),
        xi.types.Column("UpperLimit", dtype=xi.real, alias="Upper limit", manage=xi.Manage.RESULT),
    ])
    
    
    CtrSumIds: xi.types.Index(dtype=xi.string, alias="Constraints", manage=xi.Manage.RESULT)
    CtrSummary: xi.types.Series(index="CtrSumIds", dtype=xi.real, manage=xi.Manage.RESULT)

    # Constant class attribute
    DATAFILE = "shares.csv"

    @xi.ExecModeLoad(descr="Load input data and initialize all input entities.")
    def load(self):
        print("Loading data.")
        self.Shares = pd.read_csv(InsightApp.DATAFILE, index_col=['ShareIds'])
        self.ShareIds = self.Shares.index
        print("Loading finished.")

    @xi.ExecModeRun(descr="Solve problem and initialize all result entities.")
    def run(self):
        print('Starting optimization.')

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

        # Programmatic logic to compute evaluation metrics for constraints
        self.CtrSol = pd.DataFrame(columns=["Activity", "LowerLimit", "UpperLimit"])
        self.CtrSol.loc["Limit high risk shares"] = {
            "Activity": sum(self.Shares.loc[self.Shares.HighRisk, 'fraction']),
            "LowerLimit": 0,
            "UpperLimit": self.MaxHighRisk}

        self.CtrSol.loc["Limit North-American"] = {
            "Activity": sum(self.Shares.loc[self.Shares.NorthAmerica, 'fraction']),
            "LowerLimit": self.MinNorthAmerica,
            "UpperLimit": 1}

        for s in self.ShareIds:
            if self.Shares.loc[s, 'fraction'] > 0:
                self.CtrSol.loc[f"Limit per value: {s}"] = {
                    "Activity": self.Shares.loc[s, 'fraction'],
                    "LowerLimit": 0,
                    "UpperLimit": self.MaxPerShare}

        self.CtrSolIds = self.CtrSol.index
        
        self.CtrSummary = pd.Series()
        self.CtrSummary.loc["Largest position"] = max(self.Shares.fraction)
        self.CtrSummary.loc["Total high risk shares"] = sum(self.Shares.loc[self.Shares.HighRisk, 'fraction'])
        self.CtrSummary.loc["Total North-American"] = sum(self.Shares.loc[self.Shares.NorthAmerica, 'fraction'])
        self.CtrSummary.loc["Total return"] = p.attributes.objval
        
        self.CtrSumIds = self.CtrSummary.index
        
        print('Optimization finished.')


if __name__ == "__main__":
    app = xi.create_app(InsightApp)
    sys.exit(app.call_exec_modes(["LOAD", "RUN"]))
