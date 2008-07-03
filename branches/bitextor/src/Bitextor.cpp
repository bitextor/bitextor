#include "WebFile.h"

int
main (int argc, char *const *argv)
{
	WebFile wf("/home/miquel/Desktop/SEPC.html");
	cout<<wf.getLang();
	
	return 0; 
}
