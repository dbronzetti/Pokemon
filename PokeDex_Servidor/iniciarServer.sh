#!/bin/sh
#####SWAP
export POKEPORT=8080
rm /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Servidor/logServer
cd /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Servidor/Debug/
./PokeDex_Servidor -l  /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Servidor/logServer -d /home/utnso/osada-utils/discoOsada.bin
