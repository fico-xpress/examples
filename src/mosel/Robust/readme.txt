readme.txt (mmrobust examples)                                  Apr. 2014
==============================

In order to be able to run the examples in this directory, Xpress Optimizer 
must be installed and licensed. 

Introductory examples
---------------------
* Special cases
  careful_equalities.mos - robust problem with equality constraint
  careful_unbounded_uncertain.mos - robust problem with an unbounded uncertain

* Price of robustness
 cost_of_robustness.mos - calculating the cost of having uncertainty 
                          in the model

* Robust formulations of a single knapsack problem
  knapsack_basic.mos      - deterministic version of the knapsack model
  knapsack_ellipsoid.mos  - knapsack model with ellipsoidal uncertainty
  knapsack_polyhedron.mos - knapsack model with polyhedral uncertainty
  knapsack_scenario.mos   - historical data used as uncertainty scenario
  knapsack_simplebounds.mos, knapsack_simplebounds_direct.mos - 
                          knapsack model with simple bounded uncertains

* Working with nominal values
  nominalvalue_0base.mos  - simple robust model without nominal values
  nominalvalue_2base.mos  - simple robust model with a nominal valued uncertain
  nominalvalue_none.mos   - uncertainty as an addition to a coefficient
  nominalvalue_rule2.mos  - using uncertains with nominal value as coefficients
  nominalvalue_shift1.mos - basic version without nominal values
  nominalvalue_shift2.mos - defining nominal values
  nominalvalue_shift3.mos - substituting the effect of the nominal values
  nominalvalue_simple.mos - uncertainty as a coefficient

* Overlapping uncertainty
  overlapping_cardinality.mos - cardinality constraint with overlapping 
                          uncertainty sets
  overlapping_polyhedral.mos, overlapping_polyhedral2.mos -  
                          polyhedral uncertainty set with overlapping use in  
                          robust constraints

* Working with scenarios
  scenario_simple.mos -   simple robust optimization model with scenario based 
                          uncertainty
  scenario_simple_deterministic.mos - deterministic version of the simple 
                          scenario problem

Application examples
--------------------
* Robust single period portfolio allocation
  021folio_robust.mos -   'ellipsoid' uncertainty set

* Security constrained robust unit commitment
  a6electr_ro_hdem.mos (requires a6electr_ro_simple.dat) - load variation;
                          'scenario' uncertainty set
  a6electr_ro_kcont.mos (requires a6electr_ro_simple.dat) - k contigencies;
                          'cardinality' uncertainty set

* Multi-period, multi-item production planning with demand scenarios
  c2glass_robust.mos (requires c2glass_scen.dat) - 'scenario' and
                          polyhedral uncertainty sets

* Production planning with uncertain resource availability
  prodplan_robust.mos (requires prodplan_robust.dat) - 
                          'cardinality' uncertainty set

* Robust shortest path
  roadworks.mos, roadworks_ive.mos (require roads_9.dat) - 
                          'cardinality' uncertainty set

* Robust network optimization for VPN design
  vpn_design.mos (requires vpn_data.dat or vpn_data_tiny.dat) - 
                          polyhedral uncertainty set

