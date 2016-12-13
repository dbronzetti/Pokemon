#!/bin/sh
#####SWAP
export POKEPORT=8080
export POKEIP=127.0.0.1
rm /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Cliente/logCliente
cd /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Cliente/Debug/
./PokeDex_Cliente -l  /home/utnso/tp-2016-2c-CompuMundoHiperMegaRed/PokeDex_Cliente/logCliente /home/utnso/dicoFuse/ -d -s 
