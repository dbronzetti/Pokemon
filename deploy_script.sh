#!/bin/sh

DEPLOY_FOLDER="/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/"

#CREATING DESTINATION FOLDER
mkdir -p $DEPLOY_FOLDER
cd $DEPLOY_FOLDER

#DOWNLOADING TP
git clone https://github.com/sisoputnfrba/tp-2016-2c-CompuMundoHiperMegaRed.git .

mkdir libraries

#DOWNLOADING SO-COMMONS
cd $DEPLOY_FOLDER/libraries
git clone https://github.com/sisoputnfrba/so-commons-library.git

#INSTALLING SO-COMMONS
cd $DEPLOY_FOLDER/libraries/so-commons-library/
make
sudo make install 
#checkear instalacion commons
#ls -d /usr/include/commons

#DOWNLOADING AND INSTALLING NCURSES
sudo apt-get install libncurses-dev

#DOWNLOADING SO-NIVEL-LIBRARY
cd $DEPLOY_FOLDER/libraries
git clone https://github.com/sisoputnfrba/so-nivel-gui-library

#INSTALLING SO-NIVEL-LIBRARY
cd $DEPLOY_FOLDER/libraries/so-nivel-gui-library
make
sudo make install 

#DOWNLOADING SO-PKMN-UTILS
cd $DEPLOY_FOLDER/libraries
git clone https://github.com/sisoputnfrba/so-pkmn-utils

#INSTALLING SO-PKMN-UTILS
cd $DEPLOY_FOLDER/libraries/so-pkmn-utils/src
make all
sudo make install 

#DOWNLOADING LIBFUSE
#cd $DEPLOY_FOLDER/libraries
#git clone --branch fuse-3.0.0 https://github.com/libfuse/libfuse

#INSTALLING LIBFUSE
#cd $DEPLOY_FOLDER/libraries/libfuse/
#sudo apt-get install dh-autoreconf
#sudo apt-get install build-essential libtool
#sh makeconf.sh 
#./configure
#make -j8
#sudo make install

cd $DEPLOY_FOLDER/CompuMundoHiperMegaRed-commons/Debug
make

cd $DEPLOY_FOLDER/Mapa/Debug
make

cd $DEPLOY_FOLDER/Entrenador/Debug
make

cd $DEPLOY_FOLDER/PokeDex_Cliente/Debug
make

cd $DEPLOY_FOLDER/PokeDex_Servidor/Debug
make

cd $DEPLOY_FOLDER
pwd

echo "Fin deploy - ejecutar: 'cd $DEPLOY_FOLDER'"
