# Ejecutar con sudo
cp ./Debug/Consola /usr/bin/ansisop
cp ./consola.config /usr/local/share/consola.config
touch /usr/local/share/consola.log
chmod 111 /usr/bin/ansisop
chmod 666 /usr/local/share/consola.log
chmod 444 /usr/local/share/consola.config
echo "Consola -> /usr/bin/ansisop"
echo "consola.config -> /usr/local/share/consola.config"
echo "consola.log -> /usr/local/share/consola.log"
echo "Archivo de config:"
more /usr/local/share/consola.config
