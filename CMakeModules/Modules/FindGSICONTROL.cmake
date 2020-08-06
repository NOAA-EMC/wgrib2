# - Find the Control version of GSI to use for regression testing

set( NO_DEFAULT_PATH )
message("Control path is ${CONTROLPATH}")
find_file( CONTROL_EXE
    NAMES gsi.x global_gsi ${GSIEXEC}
    HINTS
        ${CONTROLPATH}
        ${CONTROLPATH}/bin
        ${CONTROLPATH}/exec
        $ENV{CONTROLPATH}
        $ENV{CONTROLPATH}/bin
        $ENV{CONTROLPATH}/exec
        $ENV{CONTROLPATH}/src
        ${CMAKE_SOURCE_DIR}/../trunk/src
        ${CMAKE_SOURCE_DIR}/../../trunk/src
        ${PROJECT_BINARY_DIR}/../build-trunk/bin
        /da/save/Michael.Lueken/svn1/build/bin
        /da/save/Michael.Lueken/svn1/src
        /gpfs/dell2/emc/modeling/noscrub/Michael.Lueken/svn1/build/bin
        /gpfs/hps3/emc/da/noscrub/Michael.Lueken/svn1/build/bin
        /gpfs/hps3/emc/da/noscrub/Michael.Lueken/svn1/src
        /scratch4/NCEPDEV/da/save/Michael.Lueken/svn1_old/build/bin
        /scratch4/NCEPDEV/da/save/Michael.Lueken/svn1/src

    ${NO_DEFAULT_PATH})

set( GSICONTROL ${CONTROL_EXE} CACHE STRING "GSI control executable for regression testing" FORCE )

find_file( ENKF_CONTROL_EXE
    NAMES enkf_gfs.x global_enkf ${ENKFEXEC}
    HINTS
        ${CONTROLPATH}
        ${CONTROLPATH}/bin
        ${CONTROLPATH}/exec
        $ENV{CONTROLPATH}
        $ENV{CONTROLPATH}/bin
        $ENV{CONTROLPATH}/exec
        ${CMAKE_SOURCE_DIR}/../trunk/src/enkf
        ${PROJECT_BINARY_DIR}/../build-trunk/bin
        $ENV{CONTROLPATH}/enkf
        $ENV{CONTROLPATH}/src/enkf
        /da/save/Michael.Lueken/svn1/build/bin
        /da/save/Michael.Lueken/svn1/src/enkf
        /gpfs/dell2/emc/modeling/noscrub/Michael.Lueken/svn1/build/bin
        /gpfs/hps3/emc/da/noscrub/Michael.Lueken/svn1/build/bin
        /gpfs/hps3/emc/da/noscrub/Michael.Lueken/svn1/src/enkf
        /scratch4/NCEPDEV/da/save/Michael.Lueken/svn1_old/build/bin
        /scratch4/NCEPDEV/da/save/Michael.Lueken/svn1/src/enkf

    ${NO_DEFAULT_PATH})

set( ENKFCONTROL ${ENKF_CONTROL_EXE} CACHE STRING "ENKF control executable for regression testing" FORCE )
