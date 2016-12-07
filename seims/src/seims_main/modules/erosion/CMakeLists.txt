MESSAGE(STATUS "    Compiling folder: erosion...")
# longterm model modules
ADD_SUBDIRECTORY(./MUSLE_AS)
ADD_SUBDIRECTORY(./IUH_SED_OL)
ADD_SUBDIRECTORY(./SEDR_SBAGNOLD)
# storm model modules, have not been updated, so uncommentd. By LJ
#ADD_SUBDIRECTORY(./SPLASHERO_PARK)
#ADD_SUBDIRECTORY(./MUSLE_I30)
#ADD_SUBDIRECTORY(./KinWavSed_OL)
#ADD_SUBDIRECTORY(./KinWavSed_CH)
