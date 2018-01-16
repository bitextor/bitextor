#include <time.h>				// clock_t, clock(), CLOCKS_PER_SEC, etc.
#include <float.h>				// FLT_MAX, etc.
#include "clustercat-cluster.h"
#include "clustercat-array.h"
#include "clustercat-math.h"

float entropy_term(const float entropy_terms[const], const unsigned int i);
double pex_remove_word(const struct cmd_args cmd_args, const word_id_t word, const word_count_t word_count, const wclass_t from_class, const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_array_t count_array, const float entropy_terms[const], const bool is_tentative_move);
double pex_move_word(const struct cmd_args cmd_args, const word_id_t word, const word_count_t word_count, const wclass_t to_class, const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_array_t count_array, const float entropy_terms[const], const bool is_tentative_move);

inline float entropy_term(const float entropy_terms[const], const unsigned int i) {
	if (i < ENTROPY_TERMS_MAX)
		return entropy_terms[i];
	else
		return i * log2f(i);
}

inline double pex_remove_word(const struct cmd_args cmd_args, const word_id_t word, const word_count_t word_count, const wclass_t from_class, const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_array_t count_array, const float entropy_terms[const], const bool is_tentative_move) {
	// See Procedure MoveWord on page 758 of Uszkoreit & Brants (2008):  https://www.aclweb.org/anthology/P/P08/P08-1086.pdf
	register double delta = 0.0;
	const unsigned int count_class = count_array[from_class];
	if (count_class > 1)
		delta = entropy_term(entropy_terms, count_class);
	const unsigned int new_count_class = count_class - word_count;
	if (new_count_class > 1)
		delta -= entropy_term(entropy_terms, new_count_class);
	//printf("rm42: word=%u, word_count=%u, from_class=%u, count_class=%u, new_count_class=%u (count_class - word_count), delta=%g\n", word, word_count, from_class, count_class, new_count_class, delta); fflush(stdout);

	if (! is_tentative_move)
		count_array[from_class] = new_count_class;

	for (unsigned int i = 0; i < word_bigrams[word].length; i++) {
		word_id_t prev_word = word_bigrams[word].predecessors[i];
		//printf(" rm43: i=%u, len=%u, word=%u, offset=%u (prev_word=%u + num_classes=%u * from_class=%u)\n", i, word_bigrams[word].length, word,  (prev_word * cmd_args.num_classes + from_class), prev_word, cmd_args.num_classes, from_class); fflush(stdout);
		const unsigned int word_class_count = word_class_counts[prev_word * cmd_args.num_classes + from_class];
		if (word_class_count > 1) // Can't do log(0); no need for 1
			delta -= entropy_term(entropy_terms, word_class_count);
		const unsigned int new_word_class_count = word_class_count - word_bigrams[word].bigram_counts[i];
		delta += entropy_term(entropy_terms, new_word_class_count);
		//printf(" rm45: word=%u (#=%u), prev_word=%u, #(<v,w>)=%u, from_class=%u, i=%u, count_class=%u, new_count_class=%u, <v,c>=<%u,%u>, #(<v,c>)=%u, new_#(<v,c>)=%u (w-c - %u), delta=%g\n", word, word_count, prev_word, word_bigrams[word].bigram_counts[i], from_class, i, count_class, new_count_class, prev_word, from_class, word_class_count, new_word_class_count, word_bigrams[word].bigram_counts[i], delta); fflush(stdout);
		//print_word_class_counts(cmd_args, model_metadata, word_class_counts);
		if (! is_tentative_move)
			word_class_counts[prev_word * cmd_args.num_classes + from_class] = new_word_class_count;

	}

	if (cmd_args.rev_alternate && (!is_tentative_move)) { // also update reversed word-class counts
		for (unsigned int i = 0; i < word_bigrams_rev[word].length; i++) {
			const word_id_t next_word = word_bigrams_rev[word].predecessors[i];
			const unsigned int word_class_rev_count = word_class_rev_counts[next_word * cmd_args.num_classes + from_class];
			const unsigned int new_word_class_rev_count = word_class_rev_count - word_bigrams_rev[word].bigram_counts[i];
			//printf(" rm47: rev_next_word=%u, rev_#(<v,c>)=%u, rev_new_#(<v,c>)=%u\n", next_word, word_class_rev_count, new_word_class_rev_count); fflush(stdout);
			//print_word_class_counts(cmd_args, model_metadata, word_class_rev_counts);
			word_class_rev_counts[next_word * cmd_args.num_classes + from_class] = new_word_class_rev_count;
		}
	}

	return delta;
}

inline double pex_move_word(const struct cmd_args cmd_args, const word_id_t word, const word_count_t word_count, const wclass_t to_class, const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_array_t count_array, const float entropy_terms[const], const bool is_tentative_move) {
	// See Procedure MoveWord on page 758 of Uszkoreit & Brants (2008):  https://www.aclweb.org/anthology/P/P08/P08-1086.pdf
	unsigned int count_class = count_array[to_class];
	if (!count_class) // class is empty
		count_class = 1;
	const unsigned int new_count_class = count_class + word_count; // Differs from paper: replace "-" with "+"
	register double delta = entropy_term(entropy_terms, count_class)  -  entropy_term(entropy_terms, new_count_class);
	//printf("mv42: word=%u, word_count=%u, to_class=%u, count_class=%u, new_count_class=%u, delta=%g, is_tentative_move=%d\n", word, word_count, to_class, count_class, new_count_class, delta, is_tentative_move); fflush(stdout);
	const float backward_lambda = 1 - cmd_args.forward_lambda;

	if (! is_tentative_move)
		count_array[to_class] = new_count_class;

	for (unsigned int i = 0; i < word_bigrams[word].length; i++) {
		word_id_t prev_word = word_bigrams[word].predecessors[i];
		//printf(" mv43: i=%u, len=%u, word=%u, offset=%u (prev_word=%u + num_classes=%u * to_class=%u)\n", i, word_bigrams[word].length, word,  (prev_word * cmd_args.num_classes + to_class), prev_word, cmd_args.num_classes, to_class); fflush(stdout);
		const unsigned int word_class_count = word_class_counts[prev_word * cmd_args.num_classes + to_class];
		if (word_class_count > 1) { // Can't do log(0); no need for 1
			if (cmd_args.unidirectional) {
				delta -= entropy_term(entropy_terms, word_class_count);
			} else {
				delta -= entropy_term(entropy_terms, word_class_count) * cmd_args.forward_lambda;
			}
		}
		const unsigned int new_word_class_count = word_class_count + word_bigrams[word].bigram_counts[i]; // Differs from paper: replace "-" with "+"
		if (new_word_class_count > 1) { // Can't do log(0)
			if (cmd_args.unidirectional) {
				delta += entropy_term(entropy_terms, new_word_class_count);
			} else {
				delta += entropy_term(entropy_terms, new_word_class_count) * cmd_args.forward_lambda;
			}
		}
		//printf(" mv45: word=%u; prev_word=%u, to_class=%u, i=%u, word_count=%u, count_class=%u, new_count_class=%u, <v,c>=<%u,%hu>, #(<v,c>)=%u, new_#(<v,c>)=%u, delta=%g\n", word, prev_word, to_class, i, word_count, count_class, new_count_class, prev_word, to_class, word_class_count, new_word_class_count, delta); fflush(stdout);
		if (! is_tentative_move)
			word_class_counts[prev_word * cmd_args.num_classes + to_class] = new_word_class_count;

	}

	if (cmd_args.rev_alternate) { // also update reversed word-class counts; reversed order of conditionals since the first clause here is more common in this function
		for (unsigned int i = 0; i < word_bigrams_rev[word].length; i++) {
			const word_id_t next_word = word_bigrams_rev[word].predecessors[i];
			const unsigned int word_class_rev_count = word_class_rev_counts[next_word * cmd_args.num_classes + to_class];
			if (word_class_rev_count > 1) // Can't do log(0); no need for 1
				if (!cmd_args.unidirectional)
					delta -= entropy_term(entropy_terms, word_class_rev_count) * backward_lambda;

			const unsigned int new_word_class_rev_count = word_class_rev_count + word_bigrams_rev[word].bigram_counts[i];
			if (new_word_class_rev_count > 1) // Can't do log(0); no need for 1
				if (!cmd_args.unidirectional)
					//delta += entropy_term(entropy_terms, word_class_rev_count) * backward_lambda;
					delta += entropy_term(entropy_terms, new_word_class_rev_count) * backward_lambda;
			//printf("word=%u, word_class_rev_count=%u, new_word_class_rev_count=%u, delta=%g\n", word, word_class_rev_count, new_word_class_rev_count, delta);
			if (!is_tentative_move)
				word_class_rev_counts[next_word * cmd_args.num_classes + to_class] = new_word_class_rev_count;
		}
	}

	return delta;
}

void cluster(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const word_count_t word_counts[const], char * word_list[restrict], wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts) {
	unsigned long steps = 0;

	if (cmd_args.class_algo == EXCHANGE  ||  cmd_args.class_algo == EXCHANGE_BROWN) { // Exchange algorithm: See Sven Martin, Jörg Liermann, Hermann Ney. 1998. Algorithms For Bigram And Trigram Word Clustering. Speech Communication 24. 19-37. http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.53.2354
		// Get initial logprob
		count_arrays_t count_arrays = malloc(cmd_args.max_array * sizeof(void *));
		init_count_arrays(cmd_args, count_arrays);
		tally_class_ngram_counts(cmd_args, model_metadata, word_bigrams, word2class, count_arrays);
		unsigned int num_classes_current = (cmd_args.num_classes > 15) && (cmd_args.refine) ? powi(2,cmd_args.refine) : cmd_args.num_classes; // Don't bother with class refinement if the number of classes is really small.  powi() is declared in clustercat-math.h

		// Build precomputed entropy terms
		float * restrict entropy_terms = malloc(ENTROPY_TERMS_MAX * sizeof(float));
		build_entropy_terms(cmd_args, entropy_terms, ENTROPY_TERMS_MAX);

		if (cmd_args.verbose > 3) {
			printf("cluster(): 42: "); long unsigned int class_sum=0; for (wclass_t i = 0; i < cmd_args.num_classes; i++) {
				printf("c_%u=%lu, ", i, (unsigned long)count_arrays[0][i]);
				class_sum += count_arrays[0][i];
			} printf("\nClass Sum=%lu; Corpus Tokens=%lu\n", class_sum, model_metadata.token_count); fflush(stdout);
		}
		double best_log_prob = training_data_log_likelihood(cmd_args, model_metadata, count_arrays, word_counts, word2class);

		if (cmd_args.verbose >= -1) {
			fprintf(stderr, "%s: Expected Steps:  %'lu (%'u word types x %'u classes x %'u cycles);  initial logprob=%g, PP=%g\n", argv_0_basename, (unsigned long)model_metadata.type_count * cmd_args.num_classes * cmd_args.tune_cycles, model_metadata.type_count, cmd_args.num_classes, cmd_args.tune_cycles, best_log_prob, perplexity(best_log_prob, (model_metadata.token_count + model_metadata.line_count))); fflush(stderr);
		}

		time_t time_start_cycles;
		time(&time_start_cycles);
		unsigned short cycle = 1; // Keep this around afterwards to print out number of actually-completed cycles
		word_id_t moved_count = 0;
		count_arrays_t temp_count_arrays = malloc(cmd_args.max_array * sizeof(void *));
		init_count_arrays(cmd_args, temp_count_arrays);
		for (; cycle <= cmd_args.tune_cycles; cycle++) {
			if (cmd_args.refine && (cycle == 4)) // Current setting forces bump to full cluster size after 3 iterations, but you can change this line and the next for a different schedule
				num_classes_current = cmd_args.num_classes;
			if ((num_classes_current != cmd_args.num_classes) && (num_classes_current > (cmd_args.num_classes / 4.0))) { // If the coarse cluster size is close to the final size, just go do the final size
				num_classes_current = cmd_args.num_classes;
				time(&time_start_cycles); // restart timer, when full clustering starts
			}

			const bool is_nonreversed_cycle = (cmd_args.rev_alternate == 0) || (cycle % (cmd_args.rev_alternate+1)); // Only do a reverse predictive exchange (using <c,v>) after every cmd_arg.rev_alternate cycles; if rev_alternate==0 then always do this part.

			clear_count_arrays(cmd_args, temp_count_arrays);
			double queried_log_prob = 0.0;
			if (model_metadata.token_count < 5e8  || cycle == cmd_args.tune_cycles || cycle == 2 || cycle == 3) { // For large training sets, only calculate PP on the interesting iterations
				tally_class_ngram_counts(cmd_args, model_metadata, word_bigrams, word2class, temp_count_arrays);
				queried_log_prob = training_data_log_likelihood(cmd_args, model_metadata, temp_count_arrays, word_counts, word2class);
			}

			// ETA stuff
			const time_t time_this_cycle = time(NULL);
			const double time_elapsed = difftime(time_this_cycle, time_start_cycles) + 7.0; // a little is added since time prediction in early cycles tend to be too optimistic
			const double time_avg_per_cycle = (time_elapsed / ((double)cycle-1));
			const unsigned int remaining_cycles = cmd_args.tune_cycles - cycle + 1;
			const double time_remaining = ( time_avg_per_cycle * remaining_cycles);
			const time_t eta = time_this_cycle + time_remaining;

			if (cmd_args.verbose >= -1) {
				if (is_nonreversed_cycle)
					fprintf(stderr, "ccat: Normal cycle %-2u", cycle);
				else
					fprintf(stderr, "ccat: Rev cycle    %-2u", cycle);
				fprintf(stderr, " C=%-3u", num_classes_current);
				if (cycle > 1) {
					fprintf(stderr, " Words moved last cycle: %.2g%% (%u/%u).", (100 * (moved_count / (float)model_metadata.type_count)), moved_count, model_metadata.type_count);
					if (cycle > 4) {
						char eta_string[300];
						strftime(eta_string, 300, "%x %X", localtime(&eta));
						fprintf(stderr, " Time left: %lim %lis.  ETA: %s", (long)time_remaining/60, ((long)time_remaining % 60), eta_string);
					}
					if (queried_log_prob) {
						if (cmd_args.ngram_input) {
							fprintf(stderr, "  LL=%g", queried_log_prob); // can't get reliable PP if input is ngram counts
						} else {
							fprintf(stderr, "  LL=%.3g PP=%g", queried_log_prob, perplexity(queried_log_prob,(model_metadata.token_count + model_metadata.line_count)));
						}
					}
					fprintf(stderr, "\n");
				}
				else if ( cmd_args.refine)
					fprintf(stderr, " Starting with %u coarse classes, for the first few cycles\n", num_classes_current);
				else
					fprintf(stderr, "\n");
				fflush(stderr);
			}
			moved_count = 0;

			//#pragma omp parallel for num_threads(cmd_args.num_threads) reduction(+:steps) // non-determinism
			for (word_id_t word_i = 0; word_i < model_metadata.type_count; word_i++) {
			//for (word_id_t word_i = model_metadata.type_count-1; word_i != -1; word_i--) {
				if (cycle < 3 && word_i < num_classes_current) // don't move high-frequency words in the first (few) iteration(s)
					continue;
				const word_count_t word_i_count = word_bigrams[word_i].headword_count;
				const wclass_t old_class = word2class[word_i];
				double scores[cmd_args.num_classes]; // This doesn't need to be private in the OMP parallelization since each thead is writing to different element in the array
				memset(scores, 0, sizeof(double) * cmd_args.num_classes);
				//const double delta_remove_word = pex_remove_word(cmd_args, word_i, word_i_count, old_class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays, true);
				//const double delta_remove_word = 0.0;  // Not really necessary
				//const double delta_remove_word_rev = 0.0;  // Not really necessary

				//printf("cluster(): 43: "); long unsigned int class_sum=0; for (wclass_t i = 0; i < cmd_args.num_classes; i++) {
				//	printf("c_%u=%u, ", i, count_arrays[0][i]);
				//	class_sum += count_arrays[0][i];
				//} printf("\nClass Sum=%lu; Corpus Tokens=%lu\n", class_sum, model_metadata.token_count); fflush(stdout);

				#pragma omp parallel for num_threads(cmd_args.num_threads) reduction(+:steps)
				for (wclass_t class = 0; class < num_classes_current; class++) { // class values range from 0 to num_classes_current-1
					if (is_nonreversed_cycle) {
						scores[class] = pex_move_word(cmd_args, word_i, word_i_count, class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays[0], entropy_terms, true);
					} else { // This is the reversed one
						scores[class] = pex_move_word(cmd_args, word_i, word_i_count, class, word_bigrams_rev, word_bigrams, word_class_rev_counts, word_class_counts, count_arrays[0], entropy_terms, true);
					}
					steps++;
				}
				//scores[old_class] -= 0.80 / cycle; // TA

				const wclass_t best_hypothesis_class = which_max(scores, num_classes_current);
				const double best_hypothesis_score = max(scores, num_classes_current);

				if (cmd_args.verbose > 1) {
					printf("Orig score for word w_«%u» using class «%hu» is %g;  Hypos %u-%u: ", word_i, old_class, scores[old_class], 1, num_classes_current);
					fprint_array(stdout, scores, num_classes_current, ","); fflush(stdout);
					//if (best_hypothesis_score > 0) { // Shouldn't happen
					//	fprintf(stderr, "Error: best_hypothesis_score=%g for class %hu > 0\n", best_hypothesis_score, best_hypothesis_class); fflush(stderr);
					//	exit(9);
					//}
				}

				if (old_class != best_hypothesis_class) { // We've improved
					moved_count++;

					if (cmd_args.verbose > 0) {
						fprintf(stderr, " Moving id=%-7u count=%-7lu %-18s %u -> %u\t(%g -> %g)\n", word_i, (unsigned long)word_bigrams[word_i].headword_count, word_list[word_i], old_class, best_hypothesis_class, scores[old_class], best_hypothesis_score); fflush(stderr);
					}
					//word2class[word_i] = best_hypothesis_class;
					word2class[word_i] = best_hypothesis_class;
					if (isnan(best_hypothesis_score)) { // shouldn't happen
						fprintf(stderr, "Error: best_hypothesis_score=%g :-(\n", best_hypothesis_score); fflush(stderr);
						exit(5);
					} else {
						best_log_prob += best_hypothesis_score;
					}

					if (is_nonreversed_cycle) {
						pex_remove_word(cmd_args, word_i, word_i_count, old_class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays[0], entropy_terms, false);
						pex_move_word(cmd_args, word_i, word_i_count, best_hypothesis_class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays[0], entropy_terms, false);
					} else { // This is the reversed one
						pex_remove_word(cmd_args, word_i, word_i_count, old_class, word_bigrams_rev, word_bigrams, word_class_rev_counts, word_class_counts, count_arrays[0], entropy_terms, false);
						pex_move_word(cmd_args, word_i, word_i_count, best_hypothesis_class, word_bigrams_rev, word_bigrams,  word_class_rev_counts, word_class_counts, count_arrays[0], entropy_terms, false);
					}
				}
			}

			//if (!moved_count) // Nothing moved in last cycle, so that's it
			//	break;
		}

		if (cmd_args.verbose >= -1) {
			fprintf(stderr, "%s: Completed steps: %'lu\n", argv_0_basename, steps); fflush(stderr);
		}
			//fprintf(stderr, "%s: Completed steps: %'lu (%'u word types x %'u classes x %'u cycles);     best logprob=%g, PP=%g\n", argv_0_basename, steps, model_metadata.type_count, num_classes_current, cycle-1, best_log_prob, perplexity(best_log_prob,(model_metadata.token_count - model_metadata.line_count))); fflush(stderr);

		if (cmd_args.class_algo == EXCHANGE_BROWN)
			post_exchange_brown_cluster(cmd_args, model_metadata, word2class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays);

		free_count_arrays(cmd_args, temp_count_arrays);
		free(temp_count_arrays);
		free_count_arrays(cmd_args, count_arrays);
		free(count_arrays);
		free(entropy_terms);

	} else if (cmd_args.class_algo == BROWN) { // Agglomerative clustering.  Stops when the number of current clusters is equal to the desired number in cmd_args.num_classes
		// "Things equal to nothing else are equal to each other." --Anon
		for (unsigned long current_num_classes = model_metadata.type_count; current_num_classes > cmd_args.num_classes; current_num_classes--) {
			for (word_id_t word_i = 0; word_i < model_metadata.type_count; word_i++) {
				float log_probs[cmd_args.num_classes];
				//#pragma omp parallel for num_threads(cmd_args.num_threads)
				for (wclass_t class = 0; class < cmd_args.num_classes; class++, steps++) {
					// Get log prob
					log_probs[class] = -1 * (class+1); // Dummy predicate
				}
				wclass_t best_class = which_maxf(log_probs, cmd_args.num_classes);
				printf("Moving w_%u to class %u\n", word_i, best_class);
			}
		}
	}
}

void print_words_and_vectors(FILE * out_file, const struct cmd_args cmd_args, const struct_model_metadata model_metadata, char * word_list[restrict], wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts) {
	count_arrays_t count_arrays = malloc(cmd_args.max_array * sizeof(void *));
	init_count_arrays(cmd_args, count_arrays);
	tally_class_ngram_counts(cmd_args, model_metadata, word_bigrams, word2class, count_arrays);

	// Build precomputed entropy terms
	float * restrict entropy_terms = malloc(ENTROPY_TERMS_MAX * sizeof(float));
	build_entropy_terms(cmd_args, entropy_terms, ENTROPY_TERMS_MAX);

	if ( ! cmd_args.print_freqs) // greedo compatible
		fprintf(out_file, "%lu %u\n", (long unsigned)model_metadata.type_count, cmd_args.num_classes); // Like output in word2vec

	for (word_id_t word_i = 0; word_i < model_metadata.type_count; word_i++) {
		const word_count_t word_i_count = word_bigrams[word_i].headword_count;
		float scores[cmd_args.num_classes]; // This doesn't need to be private in the OMP parallelization since each thead is writing to different element in the array.  We use a float here to be compatible with word2vec
		float score_min = FLT_MAX; // use this later for rescaling

		#pragma omp parallel for num_threads(cmd_args.num_threads)
		for (wclass_t class = 0; class < cmd_args.num_classes; class++) { // class values range from 0 to cmd_args.num_classes-1
			scores[class] = sqrt( -(float)pex_move_word(cmd_args, word_i, word_i_count, class, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays[0], entropy_terms, true));
			if (scores[class] < score_min)
				score_min = scores[class];
		}

		// Rescale vectors
		for (wclass_t class = 0; class < cmd_args.num_classes; class++) {
			scores[class] -= score_min;
		}

		if (cmd_args.print_freqs) // greedo compatible
			fprintf(out_file, "%lu %s ", (long unsigned) word_i_count, word_list[word_i]);
		else // word2vec compatible
			fprintf(out_file, "%s ", word_list[word_i]);

		if (cmd_args.print_word_vectors == TEXT_VEC)
			fprint_arrayf(out_file, scores, cmd_args.num_classes, " ");
		else
			fwrite(scores, sizeof(float), cmd_args.num_classes, out_file);
	}

	free_count_arrays(cmd_args, count_arrays);
	free(count_arrays);
	free(entropy_terms);
}

void post_exchange_brown_cluster(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, wclass_t word2class[], const struct_word_bigram_entry word_bigrams[const], const struct_word_bigram_entry word_bigrams_rev[const], unsigned int * restrict word_class_counts, unsigned int * restrict word_class_rev_counts, count_arrays_t count_arrays) {

	// Build precomputed entropy terms
	float * restrict entropy_terms = malloc(ENTROPY_TERMS_MAX * sizeof(float));
	build_entropy_terms(cmd_args, entropy_terms, ENTROPY_TERMS_MAX);

	// Convert word2class to an array of classes pointing to arrays of words, which will successively get merged together
	struct_class_listing class2words[cmd_args.num_classes];
	memset(class2words, 0, sizeof(struct_class_listing) * cmd_args.num_classes);
	get_class_listing(cmd_args, model_metadata, word2class, class2words); // invert word2class array so that we know what words are associated with a given class

	// Loop through classes, finding best pair of classes to merge.  Use pex_move_word() to find best pairs. Record merges separately to reduce overhead.
	for (wclass_t total_merges = 0; total_merges < cmd_args.num_classes-1; total_merges++) {
		// The scores arrays don't need to be private in the OMP parallelization, since each thread is writing to different elements in the array
		wclass_t scores_1_which[cmd_args.num_classes];
		double scores_1_val[cmd_args.num_classes];
		memset(scores_1_which, 0, sizeof(wclass_t) * cmd_args.num_classes);
		memset(scores_1_val, 0, sizeof(double) * cmd_args.num_classes);

		#pragma omp parallel for num_threads(cmd_args.num_threads)
		for (wclass_t class_1 = 0; class_1 < cmd_args.num_classes-1; class_1++) {
			const size_t scores_2_length = cmd_args.num_classes - class_1;
			double scores_2[scores_2_length];
			memset(scores_2, 0, sizeof(double) * scores_2_length);

			for (wclass_t class_2 = class_1+1; class_2 < cmd_args.num_classes; class_2++) {
				for (size_t word_offset = 0; word_offset < class2words[class_2].length; word_offset++) { // Sum of all words
					const word_id_t word = class2words[class_2].words[word_offset];
					scores_2[class_2] += pex_move_word(cmd_args, word, word_bigrams[word].headword_count, class_1, word_bigrams, word_bigrams_rev, word_class_counts, word_class_rev_counts, count_arrays[0], entropy_terms, true);
				}
				scores_1_which[class_1] = which_max(scores_2, scores_2_length);
				scores_1_val[class_1]   = max(scores_2, scores_2_length);

			}
			//const double best_pairing_val = max(scores_1_val, cmd_args.num_classes);
		}
	}

	free_class_listing(cmd_args, class2words);
	free(entropy_terms);
}


void get_class_listing(const struct cmd_args cmd_args, const struct_model_metadata model_metadata, const wclass_t word2class[const], struct_class_listing * restrict class2words) {
// Invert word2class array so that we know what words are associated with a given class

	// First pass through the word2class array to get counts of how many words are associated with a given class, then later allocate enough memory for these
	for (word_id_t word = 0; word < model_metadata.type_count; word++) {
		const wclass_t class = word2class[word];
		class2words[class].length++;
	}

	// Allocate enough memory for all words in a given class, then zero-out length values, so that we know where next word should go
	for (wclass_t class = 0; class < cmd_args.num_classes; class++) {
		class2words[class].words = malloc(sizeof(word_id_t) * class2words[class].length);
		class2words[class].length = 0;
	}

	// Now add each word to the word array, and increment local offset
	for (word_id_t word = 0; word < model_metadata.type_count; word++) {
		const wclass_t class = word2class[word];
		class2words[class].words[class2words[class].length] = word;
		class2words[class].length++; // The final value of this should be the same as before we zeroed this value out
	}
}

void free_class_listing(const struct cmd_args cmd_args, struct_class_listing * restrict class2words) {
	for (wclass_t class = 0; class < cmd_args.num_classes; class++)
		free(class2words[class].words);
}

void build_entropy_terms(const struct cmd_args cmd_args, float * restrict entropy_terms, const unsigned int entropy_terms_max) {
	entropy_terms[0] = 0.0;
	#pragma omp parallel for num_threads(cmd_args.num_threads)
	for (unsigned long i = 1; i < entropy_terms_max; i++)
		entropy_terms[i] = i * log2f(i);
}
