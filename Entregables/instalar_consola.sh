#!/bin/sh
# Copia consola.conf y el binario a /usr/local/share y /usr/bin
# Nota: Hay que setear el HOME_TP y correr con super usuario
export HOME_TP=/home/utnso/workspace/tp-2016-1c-WorstPractices
export ENTREGABLES=$HOME_TP/Entregables

if [ ! -f "$ENTREGABLES/Consola/Consola" ];
then
  echo 'ERROR! No se genero el binario en $ENTREGABLES/Consola'
else
  echo 'Moviendo binario de la consola a /usr/bin/ansisop ...'
  cp $ENTREGABLES/Consola/Consola /usr/bin/ansisop
  echo 'Archivo de config /usr/local/share/consola.config ...'
  cp $ENTREGABLES/Consola/consola.config /usr/local/share/consola.config
  echo 'crea el archivo de log...'
  touch /usr/local/share/consola.log
  echo "Consola -> /usr/bin/ansisop"
  echo "consola.config -> /usr/local/share/consola.config"
  echo "consola.log -> /usr/local/share/consola.log"
  echo "Archivo de config:"
  more /usr/local/share/consola.config
fi
