#!/bin/sh
#####SWAP
export POKEPORT=8080
export POKEIP=127.0.0.1
if [ "$#" -ne 2 ] || [ ! -d $1 ] ; then
	echo "Uso: $0 [Carpeta de Pokedex Cliente] [Punto de Montaje de FUSE]"
	echo
	exit 33
fi
POKEFOLDER=$1
FUSEFOLDER=$2
rm $POKEFOLDER/logCliente
$POKEFOLDER/Debug/PokeDex_Cliente -l $POKEFOLDER/logCliente $FUSEFOLDER -d 
