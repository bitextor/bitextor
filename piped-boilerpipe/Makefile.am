EXTRA_DIST=src

boilerpipedir = $(prefix)/share/java/

boilerpipe_DATA = piped-boilerpipe.jar

piped-boilerpipe.jar: src/main/java/com/bitextor/pipedboilerpipe/PipedBoilerpipe.java
	mvn package assembly:single
	cp target/piped-boilerpipe-1.0-SNAPSHOT-jar-with-dependencies.jar piped-boilerpipe.jar
