# ifndef TRANSLATOR_H
# define TRANSLATOR_H

# include"core.h"

int parse_key_words(
					WLS*system, const char*cmd);
char*get_prop_core(const char*prop_def_cmd);
int parse_prop_def(
						   const char*prop_def_cmd,
						   Proposition*out);
int parse_prop_inv(
				   WLS*system, 
				   const char*prop_inv_cmd);
int context_end(WLS*system);

void show_prop(Proposition*prop);
void show_current_context(WLS*system);

# endif
