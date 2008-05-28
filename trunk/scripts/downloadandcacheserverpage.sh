wget -T 30 -t 2 "$1" 2>wgettempfile
filename=$($(dirname $0)/../bin/getwgetfilename wgettempfile)
tidy -i -asxhtml -latin1 -clean --force-output true > "$2" < "$filename" 2>/tmp/tidyexit
$(dirname $0)/../bin/bt-unicode2ascii "$2" >/dev/null
cp "$2" "$filename"

#Creamos los directorios necesarios
mkdir -p "$3"

#Copiamos el fichero en la cache con los directorios ya creados
mv "$filename" "$3/"
