"""@package spatialunits
Spatial optimization of watershed BMPs based on different spatial configuration units,

   currently supported:

   - Common units such as Hydrologic response units (HRUs) and explicitly HRUs (e.g., farm units)
   - Hydrologically connected fields (Wu et al., 2018)
   - Slope position units (Qin et al., 2018)
   - Boundary-adaptive slope position units (Zhu et al., 2020)

References:

    - Qin, C.Z., Gao, H.R., Zhu, L.J., Zhu, A.X., Liu, J.Z., and Wu, H., 2018.
    Spatial optimization of watershed best management practices based on slope position units.
    Journal of Soil and Water Conservation. 73, 504–517. `Qin2018JSWC`_

    - Wu, H., Zhu, A.X., Liu, J., Liu, Y., Jiang, J., 2018.
    Best management practices optimization at watershed scale: Incorporating spatial topology
     among fields. Water Resources Management. 32(1): 155-177. `Wu2018WRM`_

    - Zhu, L.J., Qin, C.Z., and Zhu, A.X., 2020.
    Spatial optimization of watershed best management practice scenarios based on boundary-adaptive
     configuration units.
    Progress in Physical Geography: Earth and Environment. `Zhu2020PPG`_

.. _Qin2018JSWC:
    https://doi.org/10.2489/jswc.73.5.504

.. _Wu2018WRM:
    https://doi.org/10.1007/s11269-017-1801-8

.. _Zhu2020PPG:
    https://doi.org/10.1177/0309133320939002
"""
from __future__ import absolute_import

__author__ = "Liangjun Zhu, Huiran Gao"
__description__ = "Spatial optimization of watershed BMPs based on" \
                  " different spatial configuration units"

