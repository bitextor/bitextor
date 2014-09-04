EXTRA_DIST=lib src manifest.mf

boilerpipedir = $(prefix)/share/java/

boilerpipe_DATA = piped-boilerpipe.jar lib/boilerpipe-1.2.0.jar lib/nekohtml-1.9.13.jar lib/xerces-2.9.1.jar manifest.mf

piped-boilerpipe.jar:
	mkdir -p build
	javac -cp lib/boilerpipe-1.2.0.jar:lib/nekohtml-1.9.13.jar:lib/xerces-2.9.1.jar -d build/ src/pipedboilerpipe/PipedBoilerpipe.java
	jar cfm piped-boilerpipe.jar manifest.mf -C build pipedboilerpipe 
