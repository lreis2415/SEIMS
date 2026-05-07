OFTInteger=0;OFTIntegerList=1;OFTReal=2;OFTRealList=3
OFTString=4;OFTStringList=5;OFTDate=9;OFTTime=10;OFTDateTime=11
wkbPoint=1;wkbLineString=2;wkbPolygon=3;wkbMultiPoint=4
wkbMultiLineString=5;wkbMultiPolygon=6;wkbGeometryCollection=7
class _S:
    def __init__(self,*a,**k): pass
    def __getattr__(self,n): return self
    def __call__(self,*a,**k): return None
    def __iter__(self): return iter([])
    def __int__(self): return 0
def Open(*a,**k): return None
def CreateGeometryFromWkt(*a,**k): return _S()
def CreateGeometryFromJson(*a,**k): return _S()
def CreateGeometryFromGML(*a,**k): return _S()
def GetDriverByName(*a,**k): return _S()
class FieldDefn(_S): pass
class Feature(_S): pass
class Layer(_S): pass
class DataSource(_S): pass
class Geometry(_S):
    def ExportToWkt(self): return ""
    def Transform(self,*a): pass
