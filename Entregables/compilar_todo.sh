#!/bin/sh
# Copia de los proyectos de eclipse los src y la libreria Commons
# Tambien copia los makefiles de cada proyecto a su subdirectorio y lo ejecuta
# Nota: Hay que setear el HOME_TP

chmod 755 run.sh
chmod 755 compilar_todo.sh
chmod 755 limpiar_todo.sh
chmod 755 instalar_consola.sh
chmod 755 make*

export HOME_TP=/home/utnso/workspace/tp-2016-1c-WorstPractices
export COMMONS=$HOME_TP/Commons
export SWAP_TP=$HOME_TP/Swap
export NUCLEO_TP=$HOME_TP/Nucleo
export UMC_TP=$HOME_TP/UMC
export CPU_TP=$HOME_TP/CPU
export CONSOLA_TP=$HOME_TP/Consola
export ENTREGABLES=$HOME_TP/Entregables

if [ ! -d "$ENTREGABLES" ];
then
  mkdir -p $ENTREGABLES
  chown utnso:utnso $ENTREGABLES
fi
if [ ! -d "$ENTREGABLES/Commons" ];
then
  mkdir -p $ENTREGABLES/Commons
  chown utnso:utnso $ENTREGABLES/Commons
fi
if [ ! -d "$ENTREGABLES/Swap" ];
then
  mkdir -p $ENTREGABLES/Swap
  chown utnso:utnso $ENTREGABLES/Swap
fi
if [ ! -d "$ENTREGABLES/UMC" ];
then
  mkdir -p $ENTREGABLES/UMC
  chown utnso:utnso $ENTREGABLES/UMC
fi
if [ ! -d "$ENTREGABLES/Nucleo" ];
then
  mkdir -p $ENTREGABLES/Nucleo
  chown utnso:utnso $ENTREGABLES/Nucleo
fi
if [ ! -d "$ENTREGABLES/CPU" ];
then
  mkdir -p $ENTREGABLES/CPU
  chown utnso:utnso $ENTREGABLES/CPU
fi
if [ ! -d "$ENTREGABLES/Consola" ];
then
  mkdir -p $ENTREGABLES/Consola
  chown utnso:utnso $ENTREGABLES/Consola
fi

echo '\nLimpio directorio entregables...'
rm -rf $ENTREGABLES/Commons/*
rm -rf $ENTREGABLES/Swap/*
rm -rf $ENTREGABLES/UMC/*
rm -rf $ENTREGABLES/Nucleo/*
rm -rf $ENTREGABLES/CPU/*
rm -rf $ENTREGABLES/Consola/*

echo '\nCopio libCommons...'
cp -r $COMMONS/commons $ENTREGABLES/Commons

echo '\nCopio fuentes...'
cp -r $SWAP_TP/src $ENTREGABLES/Swap/src
cp -r $UMC_TP/src $ENTREGABLES/UMC/src
cp -r $NUCLEO_TP/src $ENTREGABLES/Nucleo/src
cp -r $CPU_TP/src $ENTREGABLES/CPU/src
cp -r $CONSOLA_TP/src $ENTREGABLES/Consola/src

echo '\nCopio makefiles a directorio de entregables...'
cp $ENTREGABLES/make_commons $ENTREGABLES/Commons/makefile
cp $ENTREGABLES/make_swap $ENTREGABLES/Swap/makefile
cp $ENTREGABLES/make_umc $ENTREGABLES/UMC/makefile
cp $ENTREGABLES/make_nucleo $ENTREGABLES/Nucleo/makefile
cp $ENTREGABLES/make_cpu $ENTREGABLES/CPU/makefile
cp $ENTREGABLES/make_consola $ENTREGABLES/Consola/makefile

echo '\nMakefiles con permisos de ejecucion...'
chmod 665 $ENTREGABLES/Commons/makefile
chmod 665 $ENTREGABLES/Swap/makefile
chmod 665 $ENTREGABLES/UMC/makefile
chmod 665 $ENTREGABLES/Nucleo/makefile
chmod 665 $ENTREGABLES/CPU/makefile
chmod 665 $ENTREGABLES/Consola/makefile

echo '\n\nCompilando Commons...'
cd $ENTREGABLES/Commons
make
cp $ENTREGABLES/Commons/build/libCommons.so $ENTREGABLES/Commons/libCommons.so

echo '\n\nCompilando Swap...'
cd $ENTREGABLES/Swap
make

echo '\n\nCompilando UMC...'
cd $ENTREGABLES/UMC
make

echo '\n\nCompilando Nucleo...'
cd $ENTREGABLES/Nucleo
make

echo '\n\nCompilando CPU...'
cd $ENTREGABLES/CPU
make

echo '\n\nCompilando Consola...'
cd $ENTREGABLES/Consola
make

echo '\nCopio archivos de configuracion...'
cp $SWAP_TP/swap.config $ENTREGABLES/Swap
cp $UMC_TP/umc.conf $ENTREGABLES/UMC/
cp $NUCLEO_TP/nucleo.conf $ENTREGABLES/Nucleo
cp $CPU_TP/cpu.conf $ENTREGABLES/CPU
cp $CONSOLA_TP/consola.config $ENTREGABLES/Consola

echo '\nSeteo permisos de lectura de los archivos de configuracion...'
chmod 444 $ENTREGABLES/Swap/swap.config
chmod 444 $ENTREGABLES/UMC/umc.conf
chmod 444 $ENTREGABLES/Nucleo/nucleo.conf
chmod 444 $ENTREGABLES/CPU/cpu.conf
chmod 444 $ENTREGABLES/Consola/consola.config

echo '\nCambio el propietario de los archivos binarios de root a utnso'
chown utnso:utnso $ENTREGABLES/Swap/Swap
chown utnso:utnso $ENTREGABLES/UMC/UMC
chown utnso:utnso $ENTREGABLES/Nucleo/Nucleo
chown utnso:utnso $ENTREGABLES/CPU/CPU
chown utnso:utnso $ENTREGABLES/Consola/Consola

echo '\nCorrer ./source run.sh para setear el LD_LIBRARY_PATH de la shared library Commons...'
source $ENTREGABLES/run.sh
echo '\n'
