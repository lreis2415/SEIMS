#!/usr/bin/env bash
# cmake
git fetch cmake master
git subtree pull --prefix=cmake cmake master --squash
# taudem
git fetch taudem master
git subtree pull --prefix=seims/src/taudem taudem master --squash
# metis
git fetch metis master
git subtree pull --prefix=seims/src/metis metis master --squash
# commonlibs
git fetch utilsclass master
git fetch mongoutilclass master
git fetch rasterclass master
git subtree pull --prefix=seims/src/commonlibs/UtilsClass utilsclass master --squash
git subtree pull --prefix=seims/src/commonlibs/MongoUtilClass mongoutilclass master --squash
git subtree pull --prefix=seims/src/commonlibs/RasterClass rasterclass master --squash
