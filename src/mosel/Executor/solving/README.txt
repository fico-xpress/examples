This example demonstrates using Xpress Executor to solve a Mosel model, with a client-side Mosel model
that starts a remote execution, waits for completion, and outputs the results, using the 'executor' package
included in Xpress.

To use, follow these steps:
  1. Provision yourself an Xpress Executor component in DMP
  2. Compile model/blend3c.mos
  3. Upload the model from model/blend3c.bim to the Xpress Executor component's configuration page
  4. Open mosel/blendexecutor.mos in Xpress Workbench or your favorite editor
  5. Update the DMP_EXECUTOR_URL, DMP_SOLUTION_CLIENT_ID, DMP_SOLUTION_SECRET and DMP_MANAGER_URL parameters
     to the correct values for your Xpress Executor component
  6. Run the blendexecutor model

