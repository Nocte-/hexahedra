#!/bin/bash

# Automatically set up everything you need to cross-compile for Win32 on
# Linux.  

if [ $# -ne 1 ]; then
    echo "Usage: $0 <build directory>"
    exit 1
fi

host=i686-pc-mingw32

hexahedradir="$( cd `dirname ${BASH_SOURCE[0]}` && pwd )"
mkdir -p $1
builddir="$( cd $1 && pwd )"
packagedir=$builddir/packages
libdir=$builddir/lib
mxedir=$builddir/mxe

cd $builddir
mkdir -p $packagedir
mkdir -p $libdir

sfml_version=2.0
sfml_package=SFML-$sfml_version
enet_version=1.3.8
enet_package=enet-$enet_version
luajit_package=LuaJIT-2.0.2
cryptopp_package=cryptopp-5.6.2
rhea_package=rhea
es_package=es

# Set up MXE

if [ ! -d $mxedir ] ; then
    echo "Downloading & building MXE. Go grab a drink, this could take a while."
    cd $builddir
    git clone https://github.com/mxe/mxe.git || exit 1
    cd mxe || exit 1
    make gcc boost glew lua sqlite ogg vorbis libsndfile openal freetype jpeg z bz2
fi

mxeusr=$mxedir/usr
mxeusrhost=$mxeusr/$host
toolchain_file=$mxeusrhost/share/cmake/mxe-conf.cmake

if [ ! -d $mxeusrhost ] ; then 
    echo "Path '$mxeusrhost' not found, MXE wasn't properly built." 
    exit 1 
fi 

[ -f $packagedir/$sfml_package.zip ] || wget http://github.com/LaurentGomila/SFML/archive/master.zip \
    -c -O $packagedir/$sfml_package.zip || exit 1

[ -f $packagedir/$enet_package.tar.gz ] || wget http://enet.bespin.org/download/$enet_package.tar.gz \
    -c -O $packagedir/$enet_package.tar.gz || exit 1

[ -f $packagedir/$luajit_package.tar.gz ] || wget http://luajit.org/download/$luajit_package.tar.gz \
    -c -O $packagedir/$luajit_package.tar.gz || exit 1

[ -f $packagedir/$cryptopp_package.zip ] || wget http://www.cryptopp.com/cryptopp562.zip \
    -c -O $packagedir/$cryptopp_package.zip || exit 1

[ -f $packagedir/$rhea_package.zip ] || wget http://github.com/Nocte-/rhea/archive/master.zip \
    -c -O $packagedir/$rhea_package.zip || exit 1

[ -f $packagedir/$es_package.zip ] || wget http://github.com/Nocte-/es/archive/master.zip \
    -c -O $packagedir/$es_package.zip || exit 1

# Extract the packages
cd $libdir || exit 1
[ -d $libdir/SFML-master ]          || unzip -o $packagedir/$sfml_package.zip || exit 1
[ -d $libdir/$enet_package ]        || tar zxf $packagedir/$enet_package.tar.gz || exit 1
[ -d $libdir/$luajit_package ]      || tar zxf $packagedir/$luajit_package.tar.gz || exit 1
[ -d $libdir/$cryptopp_package ]    || unzip -o $packagedir/$cryptopp_package.zip -d $cryptopp_package || exit 1
[ -d $libdir/$rhea_package-master ] || unzip -o $packagedir/$rhea_package.zip || exit 1
[ -d $libdir/$es_package-master ]   || unzip -o $packagedir/$es_package.zip || exit 1

PATH=$mxeusr/bin:$PATH


# Build SFML
if [ ! -f $libdir/$sfml_package-build/lib/libsfml-graphics.a ] ; then
    echo "Building SFML..."
    cd $libdir
    rm -rf $sfml_package-build
    mkdir -p $sfml_package-build
    cd $sfml_package-build || exit 1
    CMAKE_PREFIX_PATH=$mxeusrhost cmake ../SFML-master \
            -DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
            -DBUILD_SHARED_LIBS:BOOL=FALSE \
            -DFREETYPE_INCLUDE_DIR_ft2build=$mxeusrhost/include \
            -DFREETYPE_INCLUDE_DIR_freetype2=$mxeusrhost/include/freetype2 \
            -DOPENAL_INCLUDE_DIR=$mxeusrhost/include/AL \
            -DOPENGL_LIBRARY=$mxeusrhost/lib/libopengl32.a \
        || exit 1
    make sfml-system sfml-window sfml-network sfml-graphics || exit 1
fi

# Build LuaJIT
if [ ! -f $libdir/$luajit_package/src/lua51.dll ] ; then
    echo "Building LuaJIT..."
    cd $libdir/$luajit_package || exit 1
    make HOST_CC="gcc -m32" CROSS=$host- TARGET_SYS=Windows || exit 1
fi


# Build Crypto++
if [ ! -d $libdir/$cryptopp_package/crypto++ ] ; then
    echo "Building Crypto++..."
    cd $libdir/$cryptopp_package
    CC=$host-gcc CXX=$host-g++ AR=$host-ar make libcryptopp.a || exit 1
    $mxeusr/bin/$host-ranlib libcryptopp.a
    mkdir -p crypto++ 
    mv *.h crypto++
fi

# Build ENet
if [ ! -f $libdir/$enet_package/.libs/libenet.a ] ; then
    echo "Building ENet..."
    cd $libdir/$enet_package
    ./configure --host=$host --enable-static --disable-shared
    make
fi

# Build Rhea
if [ ! -d $libdir/$rhea_package-build ] ; then
    cd $libdir
    mkdir -p $rhea_package-build
    cd $rhea_package-build || exit 1
    cmake ../$rhea_package-master -DCMAKE_TOOLCHAIN_FILE=$toolchain_file || exit 1
    make 
fi


# Build LibES
if [ ! -d $libdir/$es_package-build ] ; then
    cd $libdir
    mkdir -p $es_package-build
    cd $es_package-build || exit 1
    cmake ../$es_package-master -DCMAKE_TOOLCHAIN_FILE=$toolchain_file || exit 1
    make 
fi

#----------------------------------------------------------------------------

# Build Hexahedra

echo "Building Hexahedra..."
cd $builddir
CMAKE_PREFIX_PATH=$mxeusrhost cmake $hexahedradir \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
    -DSFML_STATIC_LIBRARIES:BOOL=true \
    -DSFML_ROOT=$libdir/$sfml_package-build \
    -DSFML_INCLUDE_DIR=$libdir/SFML-master/include \
    -DSFML_GRAPHICS_LIBRARY=$libdir/$sfml_package-build/lib/libsfml-graphics.a  \
    -DSFML_WINDOW_LIBRARY=$libdir/$sfml_package-build/lib/libsfml-window.a  \
    -DSFML_SYSTEM_LIBRARY=$libdir/$sfml_package-build/lib/libsfml-system.a  \
    -DENET_INCLUDE_DIR=$libdir/$enet_package/include \
    -DENET_LIBRARY=$libdir/$enet_package/.libs/libenet.a \
    -DLUAJIT_INCLUDE_DIR=$libdir/$luajit_package/src \
    -DLUAJIT_LIBRARIES=$libdir/$luajit_package/src/lua51.dll \
    -DCRYPTOPP_ROOT_DIR=$libdir/$cryptopp_package/ \
    -DCRYPTOPP_INCLUDE_DIR=$libdir/$cryptopp_package/ \
    -DCRYPTOPP_LIBRARY=$libdir/$cryptopp_package/libcryptopp.a \
    -DRHEA_INCLUDE_DIR=$libdir/$rhea_package-master/ \
    -DRHEA_LIBRARY=$libdir/$rhea_package-build/rhea/librhea-s.a \
    -DES_INCLUDE_DIR=$libdir/$es_package-master/ \
    -DES_LIBRARY=$libdir/$es_package-build/es/libes-s.a \
    -DCMAKE_INSTALL_PREFIX=/tmp \
    || exit 1

make package

