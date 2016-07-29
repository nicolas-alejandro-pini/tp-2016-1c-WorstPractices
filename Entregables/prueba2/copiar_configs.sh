#!/bin/sh
# Copia consola.conf y el binario a /usr/local/share y /usr/bin
# Nota: Hay que setear el HOME_TP y correr con super usuario
export HOME_TP=/home/utnso/workspace/tp-2016-1c-WorstPractices
export ENTREGABLES=$HOME_TP/Entregables
export PRUEBA=$ENTREGABLES/prueba2

# permisos de ejecucion sobre los programas del directorio
chmod 555 *.ansisop
# permisos de lectura y escritura sobre los archivos de configuracion
chmod 666 *.conf*

# Copio conf Swap
if [ ! -f "$ENTREGABLES/Swap/Swap" ];
then
  echo 'ERROR! compilar_todo.sh no genero el binario en $ENTREGABLES/Swap/Swap'
else
  cp $PRUEBA/swap.config $ENTREGABLES/Swap
  echo 'cp $PRUEBA/swap.config $ENTREGABLES/Swap'
  chown utnso:utnso $ENTREGABLES/Swap/swap.config
fi

# Copio conf UMC
if [ ! -f "$ENTREGABLES/UMC/UMC" ];
then
  echo 'ERROR! compilar_todo.sh no genero el binario en $ENTREGABLES/UMC/UMC'
else
  cp $PRUEBA/umc.conf $ENTREGABLES/UMC
  echo 'cp $PRUEBA/umc.conf $ENTREGABLES/UMC'
  chown utnso:utnso $ENTREGABLES/UMC/umc.conf
fi

# Copio conf Nucleo
if [ ! -f "$ENTREGABLES/Nucleo/Nucleo" ];
then
  echo 'ERROR! compilar_todo.sh no genero el binario en $ENTREGABLES/Nucleo/Nucleo'
else
  cp $PRUEBA/nucleo.conf $ENTREGABLES/Nucleo
  echo 'cp $PRUEBA/nucleo.conf $ENTREGABLES/Nucleo'
  chown utnso:utnso $ENTREGABLES/Nucleo/nucleo.conf
fi

# Copio conf Swap
if [ ! -f "$ENTREGABLES/CPU/CPU" ];
then
  echo 'ERROR! compilar_todo.sh no genero el binario en $ENTREGABLES/CPU/CPU'
else
  cp $PRUEBA/cpu.conf $ENTREGABLES/CPU
  echo 'cp $PRUEBA/cpu.conf $ENTREGABLES/CPU'
  chown utnso:utnso $ENTREGABLES/CPU/cpu.conf
fi

# Copio binario y conf de la consola (si existe el binario)
if [ ! -f "$ENTREGABLES/Consola/Consola" ];
then
  echo 'ERROR! compilar_todo.sh no genero el binario en $ENTREGABLES/Consola/Consola'
else
  echo 'Moviendo binario de la consola a /usr/bin/ansisop ...'
  cp $ENTREGABLES/Consola/Consola /usr/bin/ansisop
  echo 'Archivo de config /usr/local/share/consola.config ...'
  cp $PRUEBA/consola.config /usr/local/share/consola.config
  echo 'crea el archivo de log...'
  touch /usr/local/share/consola.log
  echo 'chown utnso a consola.config , consola.log, ansisop binario'
  chown utnso:utnso /usr/local/share/consola.config
  chown utnso:utnso /usr/local/share/consola.log
  chown utnso:utnso /usr/bin/ansisop
fi

echo 'Correr el script source ../run.sh para agregar la libreria a LD_LIBRARY_PATH\n'
