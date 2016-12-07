# This file is used for make Microsoft Visual Studio Project files and compile executable files in Linux/Unix.
# This is preprocessing for SEIMS.
# First released : Liangjun Zhu
# Date           : Jun. 19, 2015
# Latest Update  : May. 12, 2016
cmake_minimum_required(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
	CMAKE_POLICY(SET CMP0015 NEW)
ENDIF(COMMAND CMAKE_POLICY)
PROJECT (SEIMS_Preprocess)
#SET (CMAKE_BUILD_TYPE "Debug")
SET (CMAKE_BUILD_TYPE "Release")

IF(WIN32)
	SET (GDAL_HOME ${CMAKE_CURRENT_SOURCE_DIR}/../include/gdal)
	SET (MONGO_HOME ${CMAKE_CURRENT_SOURCE_DIR}/../include/mongo-c-driver-0.6)
	## SET MPI
	#For MS-MPI in Windows, version v6 and above.
	set(MPIEXEC "D:/Program Files/Microsoft HPC Pack 2012/Bin/mpiexec.exe")
	# For building MPI programs the selected Visual Studio compiler is used, namely cl.exe.
	# So there is no need to set a specific MPI compiler.
	set(MPI_CXX_INCLUDE_PATH "D:/Program Files/Microsoft HPC Pack 2012/Inc")
	# Make sure the correct libraries (64-bit or 32-bit) are selected.
	# Decide between 32-bit and 64-bit libraries for Microsoft's MPI
	if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "AMD64")
		set(MS_MPI_ARCH_DIR x64)
		set(MS_MPI_PATH "D:/Program Files (x86)/Microsoft HPC Pack 2012")
	else()
		set(MS_MPI_PATH "D:/Program Files/Microsoft HPC Pack 2012")
		set(MS_MPI_ARCH_DIR x86)
	endif()
	set(MPI_CXX_INCLUDE_PATH "${MS_MPI_PATH}/Include")
	set(MPI_CXX_LIBRARIES "${MS_MPI_PATH}/Lib/i386/msmpi.lib")
	# Currently, because the GDAL is 32-bit, then only x86 MPI is supported. LJ
	#set(MPI_CXX_LIBRARIES "${MS_MPI_PATH}/Lib/${MS_MPI_ARCH_DIR}/msmpi.lib")
	set(MPI_C_INCLUDE_PATH "${MPI_CXX_INCLUDE_PATH}")
	set(MPI_C_LIBRARIES "${MPI_CXX_LIBRARIES}")
	set(MPI_INCLUDE "${MPI_CXX_INCLUDE_PATH}")
	set(MPI_LIB "${MPI_CXX_LIBRARIES}")
	## END SETTING MPI
ELSE()
	SET (CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -fopenmp -fPIC -g -ggdb")
	SET (CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -fopenmp -fPIC")
	SET (MPI_HOME /home/zhulj/mpich/)
	SET (CMAKE_CXX_COMPILER ${MPI_HOME}/bin/mpic++)
	SET (GDAL_HOME /soft/share)
	SET (MONGO_HOME ${CMAKE_CURRENT_SOURCE_DIR}/../include/mongo-c-driver-0.6-linux)
ENDIF(WIN32)

INCLUDE_DIRECTORIES(${MONGO_HOME}/include ${GDAL_HOME}/include)
LINK_DIRECTORIES(${GDAL_HOME}/lib ${MONGO_HOME}/lib)
SET (PREPROCESS_HOME ./cpp_src)
ADD_SUBDIRECTORY(${PREPROCESS_HOME})
IF(WIN32)
	SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
	SET(DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/dlls)
## copy external dll libraries to Debug and Release folder
	SET(EXTERNAL_DLLS "${DLL_DIR}/gdal15-vc9.dll" "${DLL_DIR}/libpng13-vc9.dll" "${DLL_DIR}/libexpat.dll" "${DLL_DIR}/zlib1-vc9.dll" "${DLL_DIR}/mongo-c-driver-0.6/mongoc.dll" "${DLL_DIR}/mongo-c-driver-0.6/bson.dll")
	FILE(COPY ${EXTERNAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR}/Debug)
	FILE(COPY ${EXTERNAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR}/Release)
ENDIF()
MESSAGE(STATUS "Preprocessing program for SEIMS is Ready for you!")