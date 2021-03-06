#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage:   ./build_biips.sh [-jN [-g]]"
    echo "    Where N=nb of parallel jobs."
    echo "    The options order matters."
    echo "    Use any string that does not match the option, e.g. '-', to skip an option"
fi

set -x;
# Change these variables to fit your needs
#-----------------------------------------
# use absolute instead of relative paths

if [[ "$(uname)" == "Darwin" ]]; then
    # environment variables for Mac
    export BIIPS_SRC=`pwd`
    export BIIPS_BUILD=$HOME/workspace/biips-build
    export BIIPS_ROOT=$HOME/biips
    export BOOST_ROOT=$HOME/boost_1_53_0
    export CMAKE_BUILD_TYPE=Release
    export CMAKE_GENERATOR="Unix Makefiles"
    export CMAKE_OPTIONS="-DBUILD_TESTS=OFF"
    #export CMAKE_OPTIONS="-DBUILD_TESTS=OFF -DPRINT_SYSTEM_INFO=ON"
    export CPACK_GENERATOR="PackageMaker"
    export MAKE="make $1"
    
    if [[ "$2" == "-g" ]]; then
        export BIIPS_BUILD=$HOME/workspace/biips-debug
        export CMAKE_BUILD_TYPE=Debug
    fi

else
    # environment variables for Linux
    export BIIPS_SRC=`pwd`
    export BIIPS_BUILD=/media/data/workspace/biips-build
    export BIIPS_ROOT=$HOME/biips
    export ECLIPSE=$HOME/eclipse/cpp-neon/eclipse/eclipse
    export CMAKE_ECLIPSE_VERSION=4.6
    export CMAKE_BUILD_TYPE=Release
    export CMAKE_GENERATOR="Eclipse CDT4 - Unix Makefiles"
    export CMAKE_OPTIONS="-DCMAKE_ECLIPSE_EXECUTABLE=$ECLIPSE -DCMAKE_ECLIPSE_VERSION=$CMAKE_ECLIPSE_VERSION -DCMAKE_ECLIPSE_MAKE_ARGUMENTS=$1 -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE"
    # OpenSuse: add -DBoost_USE_STATIC_LIBS=OFF
    export CPACK_GENERATOR=DEB
    # OpenSuse: use RPM
    export MAKE="make $1"
    
    if [[ "$2" == "-g" ]]; then
        export BIIPS_BUILD=/media/data/workspace/biips-debug
        # export MATLAB_ROOT=/usr/local/MATLAB/R2010b
        export CMAKE_BUILD_TYPE=Debug
    fi
fi


#-----------------------------------------

set +x; echo -n "*** Git pull? (y/N) "; read ans
if [[ $ans == "y" ]]; then set -x
    cd $BIIPS_SRC
    git pull
fi

set +x; echo -n "*** Run CMake? (y/N) "; read ans
if [[ $ans == "y" ]]; then
	echo -n "*** Clear build directory? (y/N) "; read ans
	if [[ $ans == "y" ]]; then set -x
	    rm -rf $BIIPS_BUILD
	fi
	set +x; if [ ! -d "$BIIPS_BUILD" ]; then set -x
	    mkdir $BIIPS_BUILD
	fi
    set -x
    cd $BIIPS_BUILD
    if [ -e  _CPack_Packages ]; then
        sudo rm -rf _CPack_Packages
    fi
    cmake -G"$CMAKE_GENERATOR" $CMAKE_OPTIONS -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$BIIPS_ROOT $BIIPS_SRC
fi

cd $BIIPS_BUILD

set +x; echo -n "*** Build/install Biips? (y/N) "; read ans
if [[ $ans == "y" ]]; then set -x
    rm -rf $BIIPS_ROOT; mkdir $BIIPS_ROOT
    $MAKE install

    set +x; echo -n "*** Run biipstest tests? (y/N) "; read ans
    if [[ $ans == "y" ]]; then set -x
        cd $BIIPS_BUILD/test
        $MAKE test
    fi

    set +x; echo -n "*** Run biipstestcompiler tests? (y/N) "; read ans
    if [[ $ans == "y" ]]; then set -x
        cd $BIIPS_BUILD/test_compiler
        $MAKE test
    fi

    set +x; echo -n "*** Package Biips? (y/N) "; read ans
    if [[ $ans == "y" ]]; then set -x
        cd $BIIPS_BUILD
	    $MAKE package_source
        sudo cpack -G $CPACK_GENERATOR
        sudo cpack -G TGZ

        if [[ "$(uname)" != "Darwin" ]]; then
            set +x; echo -n "*** Install Biips DEB package ? (y/N) "; read ans
            if [[ $ans == "y" ]]; then set -x
                cd $BIIPS_BUILD
                sudo dpkg -i $BIIPS_BUILD/*.deb
                sudo apt-get -f install
            fi
        fi
    fi
fi


set +x; read -p "*** Press [Enter] key to finish..."

