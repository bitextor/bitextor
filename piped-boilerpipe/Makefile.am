EXTRA_DIST=src

boilerpipedir = $(prefix)/share/java/

boilerpipe_DATA = piped-boilerpipe.jar

piped-boilerpipe.jar:
	mvn package assembly:single
	cp target/piped-boilerpipe-1.0-SNAPSHOT-jar-with-dependencies.jar piped-boilerpipe.jar
