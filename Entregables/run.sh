#!/bin/sh
# setea LD_LIBRARY_PATH , ejecuta con "source run.sh"
# Nota: Hay que setear el HOME_TP
export HOME_TP=/home/utnso/workspace/tp-2016-1c-WorstPractices
export ENTREGABLES=$HOME_TP/Entregables
export COMMONS=$ENTREGABLES/Commons
export LD_LIBRARY_PATH=$COMMONS:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH
