#!/bin/sh
# borro todo el directorio de entregables para no subirlo al repo
# Nota: Hay que setear el HOME_TP
export HOME_TP=/home/utnso/workspace/tp-2016-1c-WorstPractices
export ENTREGABLES=$HOME_TP/Entregables

echo 'Limpio directorio entregables...'
rm -rf $ENTREGABLES/Commons
rm -rf $ENTREGABLES/Swap
rm -rf $ENTREGABLES/UMC
rm -rf $ENTREGABLES/Nucleo
rm -rf $ENTREGABLES/CPU
rm -rf $ENTREGABLES/Consola
