EXTRA_DIST=src

tikadir = $(prefix)/share/java/

tika_DATA = piped-tika.jar

piped-tika.jar:
	mvn package assembly:single
	cp target/piped-tika-1.0-SNAPSHOT-jar-with-dependencies.jar piped-tika.jar
