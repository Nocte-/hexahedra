import os
import shutil
import urllib.request
from zipfile import ZipFile
import tarfile

deps = "hexahedra-deps"
zlib  = "zlib-1.2.8"
boost = "boost_1_55_0"
enet  = "enet-1.3.11"
glew  = "glew-1.10.0"
leveldb = "leveldb-1.16.0"

def extract(file):
    print("extracting {}...".format(file))
    if file[-4:] == ".zip":
        zip_file = ZipFile(file)
        zip_file.extractall()
        zip_file.close()
    else:
        tar_file = tarfile.open(file)
        tar_file.extractall()
        tar_file.close()
        
def download(url, file=None):
    if not file:
        file = url.split('/')[-1]
    if not os.path.exists(file):
        urllib.request.urlretrieve(url, file)

def replace(file, src, tgt):
    f = open(file, "r")
    contents = f.read()
    f.close()
    
    contents = contents.replace(src, tgt)
    fw = open(file, "w")
    fw.write(contents)
    fw.close()

    
def install(name, dir, url, file=None):
    if not file:
        file = url.split('/')[-1]
    if not os.path.exists(dir):
        if not os.path.exists(file):
            print("Downloading {}...".format(name))
            download(url, file)
            
        extract(file)
        
def do_cmake(name, params="", premake=None):
    master = name + "-master"
    if not os.path.exists(master):
        master = name
        
    build = name + "-build"
    if not os.path.exists(build):
        os.mkdir(build)
        
    os.chdir(build)
    if not os.path.exists("ALL_BUILD.vcxproj"):
        print("Configuring "+name+" ...")
        os.system("cmake ../" + master + " -DBOOST_ROOT=../"+boost+" "+params)
        
    if premake:
        premake()
        
    #if not os.path.exists("Debug"):
    #    print("Building "+name+" Debug ...")
    #    os.system("MSBuild.exe ALL_BUILD.vcxproj /p:Configuration=Debug")
    if not os.path.exists("Release"):        
        print("Building "+name+" Release ...")
        os.system("MSBuild.exe ALL_BUILD.vcxproj /p:Configuration=Release") 
        
    os.chdir("..")

    
def github(name, author, cmd=None):
    master = name + "-master"
    install(name, master, "http://github.com/{}/{}/archive/master.zip".format(author, name), master+".zip")
    if cmd:
        cmd()
        
    do_cmake(name)

def main():
    msbuild = shutil.which("MSBuild.exe")
    if not msbuild:
        print("Error: MSBuild.exe not found. Make sure you run the 'Visual Studio Command Prompt', not cmd.exe")
        return -1

    cmake = shutil.which("cmake.exe")
    if not cmake:
        print("Error: cmake.exe not found. Make sure you CMake installed and added to PATH.")
        return -1

    os.chdir("..")
    if not os.path.exists(deps):
        os.mkdir(deps)

    os.chdir(deps)
    root = os.getcwd()
    
    install("zlib", zlib, "http://zlib.net/zlib128.zip")
    do_cmake(zlib)

    install("Boost", boost, "http://sourceforge.net/projects/boost/files/boost/1.55.0/"+boost+".tar.gz/download", boost+".tar.gz")       
    os.chdir(boost)
    if not os.path.exists("b2.exe"):
        print("Build Bjam...")
        os.system("bootstrap.bat msvc")

    if not os.path.exists("stage"):
        print("Build Boost...")
        os.system("b2.exe toolset=msvc variant=debug,release link=static threading=multi runtime-link=static stage")
        
    os.chdir("..")
        
        
    install("SFML", "SFML-2.1", "http://www.sfml-dev.org/download/sfml/2.1/SFML-2.1-sources.zip")
    def sfml_fix_vs2013():
        for x in ['graphics', 'audio', 'network', 'system', 'window', 'main' ]:
            replace("src/SFML/{x}/sfml-{x}.vcxproj".format(x=x), '&quot";"', '')
        
    do_cmake("sfml-2.1", "-DSFML_USE_STATIC_STD_LIBS=true -DBUILD_SHARED_LIBS=false", sfml_fix_vs2013)
    
    
    install("ENet", enet, "http://enet.bespin.org/download/enet-1.3.11.tar.gz")
    os.chdir(enet)
    if not os.path.exists("enet.vcxproj"):
        print("Configuring ENet...")
        os.system("vcupgrade enet.dsp")
        
    if not os.path.exists("Release/enet.lib"):
        print("Building ENet...")
        os.system("MSBuild.exe enet.vcxproj /p:Configuration=Release")
    
    os.chdir("..")
    
    install("LuaJIT", "LuaJIT-2.0.3", "http://luajit.org/download/LuaJIT-2.0.3.tar.gz")
    if not os.path.exists("LuaJIT-2.0.3/src/lua51.lib"):
        print("Build LuaJIT...")
        os.chdir("LuaJIT-2.0.3/src")
        replace("msvcbuild.bat", "/MD", "/MT")
        os.system("msvcbuild.bat static")
        os.chdir("../..")
    
    
    install("GLEW", glew, "http://sourceforge.net/projects/glew/files/glew/1.10.0/glew-1.10.0-win32.zip/download", "glew-1.10.0-win32.zip")

    
    if not os.path.exists("cryptopp562.zip"):
        print("Downloading Crypto++...")
        download("http://www.cryptopp.com/cryptopp562.zip")
        
    if not os.path.exists("cryptopp"):
        os.mkdir("cryptopp")
        os.chdir("cryptopp")
        extract("../cryptopp562.zip")
        os.chdir("..")
        
    if not os.path.exists("cryptopp/Win32/Output/Release/cryptlib.lib"):
        print("Build Crypto++...")
        os.chdir("cryptopp")
        os.system("vcupgrade cryptlib.vcproj")
        os.system("MSBuild.exe cryptlib.vcxproj /p:Configuration=Debug")
        os.system("MSBuild.exe cryptlib.vcxproj /p:Configuration=Release")
        os.chdir("..")
    


    if not os.path.exists("glm"):
        install("GLM", "glm-0.9.5.3", "http://sourceforge.net/projects/ogl-math/files/glm-0.9.5.3/glm-0.9.5.3.zip/download", "glm-0.9.5.3.zip")

        
    def leveldbfix():
        replace("leveldb-master/CMakeLists.txt", "Boost_USE_STATIC_RUNTIME OFF", "Boost_USE_STATIC_RUNTIME ON")
        replace("leveldb-master/CMakeLists.txt", "/wd4702", "/wd4702 /wd4018 /wd4244 /MT /O2")
  
    github("leveldb", "bureau14", leveldbfix)

    github("es", "Nocte-")
    github("rhea", "Nocte-")
        
    install("HexaNoise", "hexanoise-master", "http://github.com/Nocte-/hexanoise/archive/master.zip", "hexanoise-master.zip")
    if not os.path.exists("hexanoise-build"):
        os.mkdir("hexanoise-build")
        
    if not os.path.exists("hexanoise-build/hexanoise/Release/hexanoise-s.lib"):
        os.chdir("hexanoise-build")
        if not os.path.exists("hexanoise"):
            os.mkdir("hexanoise")
        for filename in [ "tokens.cpp", "tokens.hpp", "parser.cpp", "parser.hpp" ]:
            shutil.copy("../hexanoise-master/win32/" + filename, "hexanoise")
        
        os.system('cmake ../hexanoise-master -DBOOST_INCLUDEDIR="' + os.path.join(root, "boost_1_55_0") +'" -DGLM_INCLUDE_DIR="'
                   + os.path.join(root, "glm") + '"')            
        os.system("MSBuild.exe hexanoise/hexanoise-s.vcxproj /p:Configuration=Debug") 
        os.system("MSBuild.exe hexanoise/hexanoise-s.vcxproj /p:Configuration=Release") 
        os.chdir("..")
        
    
    os.chdir("..")
    if not os.path.exists("hexahedra-build"):
        os.mkdir("hexahedra-build")
        
    os.chdir("hexahedra-build")
    build_opts = r""" 
    -DBUILD_CLIENT=true 
    -DBUILD_SERVER=true 
    -DBUILD_STATIC=true
    -DZLIB_LIBRARY={root}/{zlib}-build/Release/zlibstatic.lib
    -DZLIB_INCLUDE_DIR={root}/{zlib}
    -DBOOST_ROOT={root}/{boost}
    -DBOOST_INCLUDEDIR={root}/{boost}/boost
    -DBoost_Dir={root}/{boost}
    -DBOOST_LIBRARY_DIR={root}/{boost}/stage/lib
    -DCRYPTOPP_LIBRARY_DEBUG={root}/{cryptopp}/Win32/Output/Debug/cryptlib.lib
    -DCRYPTOPP_LIBRARY_RELEASE={root}/{cryptopp}/Win32/Output/Release/cryptlib.lib
    -DCRYPTOPP_INCLUDE_DIR={root}
    -DCRYPTOPP_ROOT_DIR={root}/{cryptopp}
    -DENET_ROOT={root}/{enet}
    -DENET_LIBRARIES={root}/{enet}/Release/enet.lib
    -DES_INCLUDE_DIR={root}/es-master
    -DES_LIBRARY={root}/es-build/es/Release/es-s.lib
    -DGLEW_INCLUDE_DIR={root}/{sfml}/extlibs/headers
    -DGLEW_LIBRARIES={root}/{glew}/lib/Release/Win32/glew32s.lib
    -DGLM_INCLUDE_DIR={root}/glm
    -DHEXANOISE_INCLUDE_DIR={root}/hexanoise-master
    -DHEXANOISE_LIBRARY={root}/hexanoise-build/hexanoise/Release/hexanoise-s.lib
    -DLEVELDB_INCLUDE_DIR={root}/leveldb-master/include
    -DLEVELDB_LIBRARIES={root}/leveldb-build/Release/leveldb.lib
    -DRHEA_INCLUDE_DIR={root}/rhea-master
    -DRHEA_LIBRARY={root}/rhea-build/rhea/Release/rhea-s.lib
    -DLUAJIT_INCLUDE_DIR={root}/{luajit}/src
    -DLUAJIT_LIBRARIES={root}/{luajit}/src/lua51.lib
    -DLUAJIT_FIND_STATIC=true
    -DSFML_STATIC_LIBRARIES=true
    -DSFML_INCLUDE_DIR={root}/{sfml}/include
    -DSFML_ROOT={root}/{sfml}-build
    -DSFML_MAIN_LIBRARY={root}/{sfml}-build/lib/Release/sfml-main.lib
    -DSFML_GRAPHICS_LIBRARY={root}/{sfml}-build/lib/Release/sfml-graphics-s.lib
    -DSFML_SYSTEM_LIBRARY={root}/{sfml}-build/lib/Release/sfml-system-s.lib
    -DSFML_WINDOW_LIBRARY={root}/{sfml}-build/lib/Release/sfml-window-s.lib
    """.format(root=root, zlib=zlib, boost=boost, sfml="sfml-2.1", glew=glew, cryptopp="cryptopp", enet="enet-1.3.11", luajit="LuaJit-2.0.3").replace("\n","")
               
    os.system("cmake ../hexahedra " + build_opts)
    
    
if __name__ == "__main__":
    main()
	
