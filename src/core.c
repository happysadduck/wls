# include<stdio.h>

# include"core.h"
# include"translator.h"
# include"input.h"
# include"alarm.h"
# include"match.h"

WLS*current_system;

static void fact_free_fn(void*p){
	pool_return(current_system->pool_for_facts, p);
}

static void prop_free_fn(void*p){
	pool_return(current_system->pool_for_props, p);
}

static void name_free_fn(void*p){
	pool_return(current_system->pool_for_names, p);
}

WLS*create_system(void){
	WLS*system=malloc(sizeof(WLS));
	current_system=system;
    system->constructed_params_cnt=0;
    system->line_cnt=0;
    system->prove_try_cnt=0;
    system->prove_achieve_cnt=0;
    system->suppose_try_cnt=0;
    system->suppose_achieve_cnt=0;
    system->error_cnt=0;
	system->pool_for_facts=
	prepare_pool(MAX_FACT_LEN, MAX_FACTS_CNT);
	system->pool_for_props=
	prepare_pool(MAX_NAME_LEN+MAX_FACT_LEN, 
				 MAX_PROPS_CNT);
	system->pool_for_names=
	prepare_pool(MAX_NAME_LEN, 
				 MAX_PARAMS_CNT+MAX_PROPS_CNT);
	system->pool_for_contexts=
	prepare_pool(sizeof(Context), MAX_DEPTH);
	system->controller=
	create_layer_manager(MAX_DEPTH);
	system->fact_set=
	create_tower(MAX_FACTS_CNT, system->controller,
				 fact_free_fn, NULL);
	system->prop_table=
	create_tower(MAX_PROPS_CNT, system->controller,
				 name_free_fn, prop_free_fn);
	system->used_params=
	create_tower(MAX_PARAMS_CNT, system->controller,
				 name_free_fn, name_free_fn);
	system->current_context=NULL;
	system->current_keyword=Empty;
	return system;
}

void destroy_system(WLS*system){
	destroy_tower(system->fact_set);
	destroy_tower(system->prop_table);
	destroy_tower(system->used_params);
	destroy_layer_manager(system->controller);
	destroy_pool(system->pool_for_contexts);
	destroy_pool(system->pool_for_facts);
	destroy_pool(system->pool_for_props);
	destroy_pool(system->pool_for_names);
	current_system=NULL;
	free(system);
}

static Proposition*copy_prop(WLS*system, Proposition*src){
    Proposition*dst=pool_alloc(system->pool_for_props);
    *dst=*src;
    return dst;
}

# define RECYCLE_CMD \
pool_return(system->pool_for_props, cmd-sizeof(Proposition));

int process_cmd(WLS*system, char*cmd){
	Context*old_context;
	switch(system->current_keyword){
	case Empty:{
        int parse_result=parse_prop_inv(system, cmd);
        if(!parse_result){
            goto error;
        }if(parse_result==-1){
            return -1;
        }
        RECYCLE_CMD;
        cmd=NULL;
        break;
    }
	case Import:{
		Proposition*new_prop=
        (Proposition*)(cmd-sizeof(Proposition));
		char*new_prop_name=
		pool_alloc(system->pool_for_names);
        CHECK_NAME(new_prop_name);
		if(!parse_prop_def(cmd, new_prop)){
            RAISE_ERROR(
                        "incorrect proposition definition structure", system);
			goto error;
		}
		cmd=NULL;
		token_to_str(&new_prop->name, new_prop_name);
		tower_insert(system->prop_table,
					 new_prop_name, new_prop);
		new_prop=NULL;
		break;
	}
	case Given:{
		tower_insert(system->fact_set, cmd, NULL);
        cmd=NULL;
		break;
	}
	case Prove:{
		Proposition*to_be_prove
		=pool_alloc(system->pool_for_props);
		Context*new_context=
		pool_alloc(system->pool_for_contexts);
		int i;
        char*cmd_actual=(char*)(to_be_prove+1);
        char*cmd_core=get_prop_core(cmd);
        CHECK_CONTEXT(new_context);
        CHECK_PROP(to_be_prove);
        memcpy(cmd_actual, cmd, cmd_core-cmd);
        CARPET_REPLACE_STR(system->used_params,
                           cmd_core, cmd_actual+(cmd_core-cmd));
		if(!parse_prop_def(cmd_actual, to_be_prove)){
			goto error;
		}
        RECYCLE_CMD;
		cmd=NULL;
		new_context->behavior='p';
		new_context->parent=system->current_context;
		new_context->start_value=to_be_prove;
        new_context->refer_value=copy_prop(system, to_be_prove);
		system->current_context=new_context;
		tower_add_layer(system->controller);
		if(to_be_prove->type=='e'){
            system->current_keyword=Empty;
			break;
		}
		for(i=0; i<to_be_prove->conditions.cnt; i++){
			Token*condition=to_be_prove->conditions.tokens+i;
			char*condition_in_str
			=pool_alloc(system->pool_for_facts);
            CHECK_FACT(condition_in_str);
			token_to_str(condition, condition_in_str);
			tower_insert
			(system->fact_set, condition_in_str, NULL);
		}
		system->current_keyword=Empty;
		break;
	}
	case Suppose:{
		char*new_fact=pool_alloc(system->pool_for_facts);
		Context*new_context;
        CHECK_FACT(new_fact);
        CARPET_REPLACE_STR(system->used_params, cmd, new_fact);
        RECYCLE_CMD;
        cmd=NULL;
		new_context=
		pool_alloc(system->pool_for_contexts);
        CHECK_CONTEXT(new_context);
		new_context->behavior='s';
		new_context->start_value=new_fact;
		new_context->parent=system->current_context;
		system->current_context=new_context;
		tower_add_layer(system->controller);
		tower_insert(system->fact_set, new_fact, NULL);
		system->current_keyword=Empty;
		break;
	}
	default:
		break;
	}
    while(context_end(system)){
        tower_cut_layer(system->controller);
        if(system->current_context->behavior=='p'){
            Proposition*proved_prop=(Proposition*)
            system->current_context->start_value;
            char*name_in_str=pool_alloc(system->pool_for_names);
            char*core_in_str=pool_alloc(system->pool_for_facts);
            CHECK_NAME(name_in_str)
            CHECK_FACT(core_in_str)
            token_to_str(&proved_prop->name, name_in_str);
            token_to_str(&proved_prop->prop_core, core_in_str);
            tower_insert
            (system->prop_table, name_in_str, proved_prop);
            tower_insert(system->fact_set, core_in_str, NULL);
            system->prove_achieve_cnt++;
            SHOW_INFO("successfully proved", system);
        }else if(system->current_context->behavior=='s'){
            char*nagative_suppose=
            pool_alloc(system->pool_for_facts);
            CHECK_FACT(nagative_suppose)
            memcpy(nagative_suppose, 
                   NAGATION_PREFIX, strlen(NAGATION_PREFIX));
            nagative_suppose[strlen(NAGATION_PREFIX)]='(';
            memcpy(nagative_suppose+sizeof(NAGATION_PREFIX),
                   system->current_context->start_value, 
                   strlen(system->current_context->start_value));
            nagative_suppose[sizeof(NAGATION_PREFIX)+
                strlen(system->current_context->start_value)]=')';
            nagative_suppose[sizeof(NAGATION_PREFIX)+1+
                strlen(system->current_context->start_value)]=0;
            tower_insert(system->fact_set, nagative_suppose, NULL);
            system->suppose_achieve_cnt++;
            SHOW_INFO("successfully falsificate", system);
        }
        old_context=system->current_context;
        system->current_context=
        system->current_context->parent;
        pool_return
        (system->pool_for_contexts, old_context);
    }
	return 1;
	
	error:
	pool_return(
				system->pool_for_props,
				cmd-sizeof(Proposition));
	cmd=NULL;
    return 1;
}
