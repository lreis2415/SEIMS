from seims.preprocess.utility import read_data_items_from_txt
from seims.pygeoc.pygeoc.raster.raster import RasterUtilClass
from seims.pygeoc.pygeoc.utils.utils import StringClass


def main():
    rasterf = r'C:\z_data\ChangTing\seims_models\model_youwuzhen_10m_longterm\OUTPUT0\SED_OL_SUM.tif'
    r = RasterUtilClass.read_raster(rasterf)
    rasterf2 = r'C:\z_data\ChangTing\seims_models\model_youwuzhen_10m_longterm\OUTPUT0\SOER_SUM.tif'
    r2 = RasterUtilClass.read_raster(rasterf2)
    print ('SED_OL sum: %s, SOER sum: %f' % (r.get_sum(), r2.get_sum()))
    sedf = r'C:\z_data\ChangTing\seims_models\model_youwuzhen_10m_longterm\OUTPUT0\sed.txt'
    sed_data = read_data_items_from_txt(sedf)
    sed_sum = 0.
    for item in sed_data:
        item = StringClass.split_string(item[0], ' ', True)
        if len(item) < 3:
            continue
        sed_sum += float(item[2])
    print (sed_sum)


if __name__ == '__main__':
    main()
