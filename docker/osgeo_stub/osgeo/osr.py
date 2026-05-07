OAMS_TRADITIONAL_GIS_ORDER=0
class _S:
    def __init__(self,*a,**k): pass
    def __getattr__(self,n): return self
    def __call__(self,*a,**k): return None
    def __int__(self): return 0
class SpatialReference(_S):
    def ImportFromEPSG(self,*a): return 0
    def ExportToWkt(self): return ""
    def SetAxisMappingStrategy(self,*a): pass
    def IsSame(self,*a): return True
class CoordinateTransformation(_S):
    def TransformPoint(self,*a): return (0.,0.,0.)
