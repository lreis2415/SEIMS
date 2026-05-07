GDT_Unknown=0;GDT_Byte=1;GDT_UInt16=2;GDT_Int16=3
GDT_UInt32=4;GDT_Int32=5;GDT_Float32=6;GDT_Float64=7
GDT_CInt16=8;GDT_CInt32=9;GDT_CFloat32=10;GDT_CFloat64=11
GA_ReadOnly=0;GA_Update=1;GRA_NearestNeighbour=0;GRA_Bilinear=1
class _S:
    def __init__(self,*a,**k): pass
    def __getattr__(self,n): return self
    def __call__(self,*a,**k): return None
    def __iter__(self): return iter([])
    def __int__(self): return 0
    def __float__(self): return 0.0
def GetDriverByName(n): return _S()
def Open(*a,**k): return None
def UseExceptions(): pass
def AllRegister(): pass
def GetLastErrorMsg(): return ""
Dataset=_S;Band=_S;Driver=_S
