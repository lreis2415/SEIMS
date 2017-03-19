# SEIMS wiki
git fetch wiki master
git subtree pull --prefix=doc/wiki wiki --squash
# pygeoc
git fetch pygeoc master
git subtree pull --prefix=seims/pygeoc pygeoc --squash
# taudem
git fetch taudem master
git subtree pull --prefix=seims/src/taudem taudem --squash
# metis
git fetch metis master
git subtree pull --prefix=seims/src/metis metis --squash
# commonlibs
git fetch utilsclass master
git fetch mongoutilclass master
git fetch rasterclass master
git subtree pull --prefix=seims/src/commonlibs/UtilsClass utilsclass --squash
git subtree pull --prefix=seims/src/commonlibs/MongoUtilClass mongoutilclass --squash
git subtree pull --prefix=seims/src/commonlibs/RasterClass rasterclass --squash
