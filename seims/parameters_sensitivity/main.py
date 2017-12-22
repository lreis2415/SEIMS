from SALib.sample import morris as morris_spl
from SALib.analyze import morris as morris_alz

from scoop import futures

# 1. Read param_rng.def file
# 2. Sampling and write to a single file and MongoDB 'PARAMETERS' collection
# 3. Run SEIMS model based on SCOOP, and write objective output variables
# 4. Calculate Morris elementary effects
