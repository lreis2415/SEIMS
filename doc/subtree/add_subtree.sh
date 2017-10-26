# SEIMS wiki
git remote add -f wiki https://github.com/lreis2415/SEIMS2017.wiki.git
git subtree add --prefix=doc/wiki wiki master --squash
# pygeoc
git remote add -f pygeoc git@github.com:lreis2415/PyGeoC.git -m master
git subtree add --prefix=seims/pygeoc pygeoc master --squash
# taudem
git remote add -f taudem git@github.com:lreis2415/TauDEM_ext.git -m master
git subtree add --prefix=seims/src/taudem taudem master --squash
# metis
git remote add -f metis git@github.com:lreis2415/metis.git -m master
git subtree add --prefix=seims/src/metis metis master --squash
# commonlibs
git remote add -f utilsclass git@github.com:lreis2415/UtilsClass.git -m master
git remote add -f mongoutilclass git@github.com:lreis2415/MongoUtilClass.git -m master
git remote add -f rasterclass git@github.com:lreis2415/RasterClass.git -m master
git subtree add --prefix=seims/src/commonlibs/UtilsClass utilsclass master --squash
git subtree add --prefix=seims/src/commonlibs/MongoUtilClass mongoutilclass master --squash
git subtree add --prefix=seims/src/commonlibs/RasterClass rasterclass master --squash
