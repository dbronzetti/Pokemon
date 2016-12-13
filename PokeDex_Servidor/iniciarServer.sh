#!/bin/sh
#####SWAP
export POKEPORT=8080
if [ "$#" -ne 2 ] || [ ! -d $1 ] ; then
	echo "Uso: $0 [Carpeta de Pokedex Server] [Disco OSADA]"
        echo
        exit 33
fi
POKEFOLDER=$1
OSADABIN=$2
rm $POKEFOLDER/logServer
$POKEFOLDER/Debug/PokeDex_Servidor -l $POKEFOLDER/logServer -d $OSADABIN 
