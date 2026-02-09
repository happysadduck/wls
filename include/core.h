# ifndef CORE_H
# define CORE_H

# include"hash.h"
# include"token.h"
# include"pool.h"

typedef struct Context{
	struct Context*parent;
	void*start_value;
    void*refer_value;
	char behavior;
}Context;

typedef struct{
	Token name;
	Token prop_core;
	const char*prop_def;
	char type;
	Tokens params;
	Tokens conditions;
	Tokens conclusions;
}Proposition;

typedef enum{
	Empty,
	Import,
	Given,
	Prove,
	Suppose
}keywordType;

typedef struct{
    int constructed_params_cnt;
    int line_cnt;
	keywordType current_keyword;
    int prove_try_cnt;
    int prove_achieve_cnt;
    int suppose_try_cnt;
    int suppose_achieve_cnt;
    int error_cnt;
	Context*current_context;
	HashTower*fact_set;
	HashTower*used_params;
	HashTower*prop_table;
	LayerManager*controller;
	Pool*pool_for_facts;
	Pool*pool_for_props;
	Pool*pool_for_names;
	Pool*pool_for_contexts;
}WLS;

WLS*create_system(void);
void destroy_system(WLS*system);

int process_cmd(WLS*system, char*cmd);

void show_prop(Proposition*prop);

void show_current_context(WLS*system);

# endif
