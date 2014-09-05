EXTRA_DIST=lib src manifest.mf

tikadir = $(prefix)/share/java/

tika_DATA = piped-tika.jar lib/tika-app-1.5.jar manifest.mf

piped-tika.jar:
	mkdir -p build
	javac -cp lib/tika-app-1.5.jar -d build/ src/pipedtika/PipedTika.java
	jar cfm piped-tika.jar manifest.mf -C build pipedtika 
