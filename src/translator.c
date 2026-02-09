# include<stdio.h>

# include"translator.h"
# include"match.h"
# include"alarm.h"

# define MAKE_CONSTRUCTED_PARAM(system, out) \
do{\
char num[12];\
sprintf(num, "%d", system->constructed_params_cnt++);\
strcpy(out, "CONSTRUCTED");\
strcat(out, num);\
}while(0)

static void fact_show_fn(void*value){
	printf("%s", (char*)value);
}

static void prop_show_fn(void*value){
	printf("\nProposition:\n");
	show_prop((Proposition*)value);
}

void show_prop(Proposition*prop){
    int i;
    printf("name: ");
    print_token(prop->name);
    printf("core: ");
    print_token(prop->prop_core);
    printf("type: ");
    printf("%c\n", (prop->type=='u')? 'u': 'e');
    printf("params: \n");
    for(i=0; i<prop->params.cnt; i++){
        printf("%d: ", (int)i+1);
        print_token(prop->params.tokens[i]);
    }
    printf("conditions: \n");
    for(i=0; i<prop->conditions.cnt; i++){
        printf("%d: ", (int)i+1);
        print_token(prop->conditions.tokens[i]);
    }
    printf("conclusions: \n");
    for(i=0; i<prop->conclusions.cnt; i++){
        printf("%d: ", (int)i+1);
        print_token(prop->conclusions.tokens[i]);
    }
}

void show_current_context(WLS*system){
    Context*c=system->current_context;
    if(!c){
        printf("no context\n");
        return;
    }
    if(c->behavior=='p'){
        printf("proving prop:\n");
        show_prop((Proposition*)(c->start_value));
    }else if(c->behavior=='s'){
        printf("supposing fact:\n");
        printf("%s\n", (char*)c->start_value);
    }else{
        printf("broken context\n");
    }
}

int context_end(WLS*system){
    Context*c=system->current_context;
    if(!c){
        return 0;
    }
    if(c->behavior=='p'){
        Proposition*prop=(Proposition*)c->refer_value;
        int i;
        char core_in_str[MAX_FACT_LEN];
        token_to_str(&prop->prop_core, core_in_str);
        if(is_in_tower(system->fact_set, core_in_str)){
            return 1;
        }
        for(i=0; i<prop->conclusions.cnt; i++){
            char conclusion_in_str[MAX_FACT_LEN];
            token_to_str(prop->conclusions.tokens+i, 
                         conclusion_in_str);
            if(!is_in_tower(system->fact_set, 
                            conclusion_in_str)){
                return 0;
            }
        }
        return 1;
    }else if(c->behavior=='s'){
        if(is_in_tower(system->fact_set, CONFLICT_MESSAGE)){
            return 1;
        }
    }
    return 0;
}

char*get_prop_core(const char*prop_def_cmd){
    const char*temp1, *temp2;
    temp1=strstr_with_skipping_brackets(prop_def_cmd, "(any:");
    temp2=strstr_with_skipping_brackets(prop_def_cmd, "(exist:");
    if(temp1){
        return (char*)temp1;
    }return (char*)temp2;
}

int parse_prop_def(const char*prop_def_cmd, 
					Proposition*prop){
	Token name, params, conditions, conclusions;
	const char*temp1, *temp2;
	temp1=strstr_with_skipping_brackets(prop_def_cmd, "(any:");
	temp2=strstr_with_skipping_brackets(prop_def_cmd, "(exist:");
	if(temp1 && temp2){
		if(temp1<temp2){
			prop->type='u';
		}else{
			prop->type='e';
			temp1=temp2;
		}
	}else if(temp1){
		prop->type='u';
	}else if(temp2){
		prop->type='e';
		temp1=temp2;
	}else{
		goto error;
	}
	name.start=prop_def_cmd;
	name.len=temp1-prop_def_cmd;
	prop->prop_core.start=temp1;
	temp1=temp2=NULL;
	params.start=
	name.start+((prop->type=='u')? 
				strlen("(any:"): strlen("(exist:"))
	+name.len;
	conditions.start=
	strstr_with_skipping_brackets(
								  params.start, ")");
	if(!conditions.start){
		goto error;
	}conditions.start+=strlen(")");
    params.len=(conditions.start-params.start)-strlen(")");
	conclusions.start=
	strstr_with_skipping_brackets(
								  conditions.start, ":");
	if(!conclusions.start){
		goto error;
	}conclusions.start+=strlen(":");
	conditions.len=(
					conclusions.start-conditions.start)-strlen(":");
	temp1=conclusions.start+strlen(conclusions.start);
	conclusions.len=(temp1-conclusions.start);
	prop->prop_core.len=temp1-prop->prop_core.start;
	temp1=NULL;
	remove_outer_brackets(&name);
	remove_outer_brackets(&params);
	remove_outer_brackets(&conditions);
	remove_outer_brackets(&conclusions);
	token_to_tokens(params, ",", &(prop->params));
	token_to_tokens(conditions, ",", &(prop->conditions));
	token_to_tokens(conclusions, ",", &(prop->conclusions));
	prop->name=name;
	prop->prop_def=prop_def_cmd;
	return 1;
	error:
	return 0;
}

int parse_prop_inv(WLS*system, const char*prop_inv_cmd){
	char*end_of_name=(char*)
	strstr_with_skipping_brackets(prop_inv_cmd, "(");
	char*end_of_cmd=
	(char*)prop_inv_cmd+strlen(prop_inv_cmd);
	Proposition*prop;
	Token new_params_uncut;
	Tokens new_params_raw;
    Tokens new_params;
    char buf[MAX_FACT_LEN*MAX_TOKENS];
	int i, off=0;
    if(!end_of_name){
        RAISE_ERROR("not legal prop invocation structure", system);
        return 0;
    }
	*end_of_name=0;
	prop=(Proposition*)
	tower_find(system->prop_table, prop_inv_cmd);
	if(!prop){
        RAISE_ERROR("no such proposition", system);
		return 0;
	}
	*end_of_name='(';
	new_params_uncut.start=end_of_name;
	new_params_uncut.len=end_of_cmd-end_of_name;
	remove_outer_brackets(&new_params_uncut);
	token_to_tokens(new_params_uncut, ",", &new_params_raw);
	if(prop->type=='e'){
		for(i=0; i<new_params_raw.cnt; i++){
			Token*param=new_params_raw.tokens+i;
			char*param_in_str=
			pool_alloc(system->pool_for_names);
            char*actual_in_str=
            pool_alloc(system->pool_for_names);
            CHECK_NAME(param_in_str);
            CHECK_NAME(actual_in_str);
			token_to_str(param, param_in_str);
            MAKE_CONSTRUCTED_PARAM(system, actual_in_str);
            if(!tower_insert
               (system->used_params, param_in_str, actual_in_str)){
                RAISE_ERROR("repetition define", system);
                return 0;
            }
            new_params.tokens[i].start=actual_in_str;
            new_params.tokens[i].len=strlen(actual_in_str);
		}new_params.cnt=new_params_raw.cnt;
        if(system->current_context &&
           system->current_context->behavior=='p'){
            Proposition*context_prop=(Proposition*)
            (system->current_context->refer_value);
            if(context_prop->type=='e'){
                Proposition*revised_prop=pool_alloc
                (system->pool_for_props);
                char*revised_def=(char*)(revised_prop+1);
                char*old_def=(char*)(context_prop+1);
                char*old_prop_core=get_prop_core(old_def);
                CHECK_PROP(revised_prop);
                memcpy(revised_def, old_def, 
                       old_prop_core-old_def);
                CARPET_REPLACE_STR(system->used_params,
                                   old_prop_core, 
                                   revised_def+(old_prop_core-old_def));
                parse_prop_def(revised_def, revised_prop);
                system->current_context->refer_value=revised_prop;
                pool_return(system->pool_for_props, context_prop);
            }
        }
		goto output;
	}
    for(i=0; i<new_params_raw.cnt; i++){
        new_params.tokens[i].start=buf+off;
        carpet_replace(system->used_params,
                       new_params_raw.tokens+i,
                       new_params.tokens+i);
        off+=new_params.tokens[i].len;
    }new_params.cnt=new_params_raw.cnt;
	for(i=0; i<prop->conditions.cnt; i++){
		Token*condition=prop->conditions.tokens+i;
		char*condition_in_str=(char*)condition->start;
		char*condition_tail=condition_in_str+condition->len;
		char helper=*condition_tail;
        char condition_actual[MAX_FACT_LEN];
		*condition_tail=0;
		batch_replace(&prop->params, 
					  &new_params, 
					  condition_in_str, 
					  condition_actual);
		*condition_tail=helper;
		if(!is_in_tower(system->fact_set, condition_actual)){
            char error_info[]="lack necessary premise: ";
            char error_details[MAX_FACT_LEN];
            sprintf(error_details, "%s", condition_actual);
            strcat(error_info, error_details);
            RAISE_ERROR(error_info, system);
			return 0;
		}
	}
	output:
	for(i=0; i<prop->conclusions.cnt; i++){
		Token*conclusions=prop->conclusions.tokens+i;
		char*conclusion_in_str=(char*)conclusions->start;
		char*conclusion_tail=conclusion_in_str+conclusions->len;
		char helper=*conclusion_tail;
		char*conclusion_actual=
		pool_alloc(system->pool_for_facts);
        CHECK_FACT(conclusion_actual);
		*conclusion_tail=0;
		batch_replace(&prop->params, 
					  &new_params, 
					  conclusion_in_str, 
					  conclusion_actual);
		*conclusion_tail=helper;
		tower_insert(system->fact_set, conclusion_actual, NULL);
	}
	return 1;
}

int parse_key_words(WLS*system,
					 const char*key_word){
	if(strcmp(key_word, IMPORT_KEY)==0){
        if(system->current_context){
            RAISE_ERROR("mixed import/given"
                        "& prove/suppose is forbidden", system);
            return 1;
        }
		system->current_keyword=Import;
		return 1;
	}if(strcmp(key_word, GIVEN_KEY)==0){
        if(system->current_context){
            RAISE_ERROR("mixed import/given"
                        "& prove/suppose is forbidden", system);
            return 1;
        }
		system->current_keyword=Given;
		return 1;
	}if(strcmp(key_word, PROVE_KEY)==0){
		system->current_keyword=Prove;
        system->prove_try_cnt++;
		return 1;
	}if(strcmp(key_word, SUPPOSE_KEY)==0){
		system->current_keyword=Suppose;
        system->suppose_try_cnt++;
		return 1;
	}if(strcmp(key_word, DEBUG_KEY)==0){
		printf("\n========================="
			   "DEBUG"
			   "=========================\n");
		printf("===facts===\n");
		show_tower(system->fact_set, fact_show_fn);
		printf("\n");
		printf("===props===\n");
		show_tower(system->prop_table, prop_show_fn);
		printf("\n");
        printf("===params===\n");
        show_tower(system->used_params, fact_show_fn);
        printf("\n");
		printf("===current context===\n");
		show_current_context(system);
        printf("========================="
               "====="
               "=========================\n\n");
		return 1;
	}if(strcmp(key_word, QUIT_KEY)==0){
		return -1;
	}
	return 0;
}
