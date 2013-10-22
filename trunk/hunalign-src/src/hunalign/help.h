#include <string>

std::string helpString = "Usage (either):\n\
    alignerTool [ common_arguments ] [ -hand=hand_align_file ] dictionary_file source_text target_text\n\
\n\
or:\n\
    alignerTool [ common_arguments ] -batch dictionary_file batch_file\n\
\n\
where\n\
common_arguments ::= [ -text ] [ -bisent ] [ -utf ] [ -cautious ] [ -realign [ -autodict=filename ] ]\n\
    [ -thresh=n ] [ -ppthresh=n ] [ -headerthresh=n ] [ -topothresh=n ]\n\
\n\
Arguments:\n\
\n\
-text\n\
	The output should be in text format, rather than the default (numeric) ladder format.\n\
\n\
-bisent\n\
	Only bisentences (one-to-one alignment segments) are printed. In non-text mode, their\n\
	starting rung is printed.\n\
\n\
-cautious\n\
	In -bisent mode, only bisentences for which both the preceeding and the following\n\
	segments are one-to-one are printed. In the default non-bisent mode, only rungs\n\
	for which both the preceeding and the following segments are one-to-one are printed.\n\
\n\
-hand=file\n\
	When this argument is given, the precision and recall of the alignment is calculated\n\
	based on the manually built ladder file. Information like the following is written\n\
	on the standard error: \n\
	53 misaligned out of 6446 correct items, 6035 bets.\n\
	Precision: 0.991218, Recall: 0.928017\n\
	\n\
        Note that by default, 'item' means rung. The switch -bisent also changes the semantics\n\
	of the scoring from rung-based to bisentence-based and in this case 'item' means bisentences.\n\
	See File formats about the format of this input align file.\n\
\n\
-realign\n\
	If this option is set, the alignment is built in three phases.\n\
	After an initial alignment, the algorithm heuristically adds items\n\
	to the dictionary based on cooccurrences in the identified bisentences.\n\
	Then it re-runs the alignment process based on this larger dictionary.\n\
	This option is recommended to achieve the highest possible alignment quality.\n\
	It is not set by default because it approximately triples the running time\n\
	while the quality improvement it yields are typically small.\n\
\n\
-autodict=filename\n\
	The dictionary built during realign is saved to this file. By default, it is not saved.\n\
\n\
-utf\n\
	The system uses the character counts of the sentences as information for the\n\
	pairing of sentences. By default, it assumes one-byte character encoding such\n\
	as ISO Latin-1 when calculating these counts. If our text is in UTF-8 format,\n\
	byte counts and character counts are different, and we must use the -utf switch\n\
	to force the system to properly calculate character counts.\n\
	Note: UTF-16 input is not supported.\n\
\n\
Postfiltering options:\n\
There are various postprocessors which remove implausible rungs based on various heuristics.\n\
\n\
-thresh=n\n\
	Don't print out segments with score lower than n/100.\n\
\n\
-ppthresh=n\n\
	Filter rungs with less than n/100 average score in their vicinity.\n\
\n\
-headerthresh=n\n\
	Filter all rungs at the start and end of texts until finding a reliably\n\
	plausible region.\n\
\n\
-topothresh=n\n\
	Filter rungs with less than n percent of one-to-one segments in their vicinity.\n\
\n\
";
