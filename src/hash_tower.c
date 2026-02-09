# include<string.h>
# include<stdio.h>

# include"hash.h"
# include"pool.h"
# include"token.h"

# define HASH(str, bucket_cnt, out)\
do{\
char*p=(char*)str;\
int hash_value=0, mask=bucket_cnt-1;\
while(*p){\
hash_value=hash_value*31+*p++;\
}\
*out=hash_value&mask;\
}while(0)

typedef struct TowerNode{
	char*key;
	void*data;
	int layer_create;
	struct TowerNode*next;
}TowerNode;

typedef struct Layer{
	int lv;
	struct Layer*prv_lv;
}Layer;

struct LayerManager{
	int layer_max;
	Layer*current;
	Pool*pool_for_layers;
	Set*using_lvs;
};

struct HashTower{
	int bucket_cnt;
	TowerNode**bucket;
	Pool*pool_for_nodes;
	dict_free_fn key_free_fn;
	dict_free_fn value_free_fn;
	LayerManager*manager;
};

static void default_free_fn(void*val){
	val=val;
}

LayerManager*create_layer_manager(
								  int level_cnt){
	LayerManager*m=malloc
	(sizeof(LayerManager));
	m->pool_for_layers=
	prepare_pool(sizeof(Layer), level_cnt);
	m->current=(Layer*)pool_alloc(m->pool_for_layers);
	m->current->lv=0;
	m->layer_max=0;
	m->current->prv_lv=NULL;
	m->using_lvs=create_set(level_cnt);
	set_insert(m->using_lvs,m->current->lv);
	return m;
}

HashTower*create_tower(int n, LayerManager*m,
					   dict_free_fn key_free_fn,
					   dict_free_fn value_free_fn){
	int bucket_cnt=1;
	HashTower*t;
	while(bucket_cnt<n)
		bucket_cnt<<=1;
	t=(HashTower*)calloc(sizeof(HashTower)+
			 bucket_cnt*sizeof(TowerNode*), 1);
	t->bucket=(TowerNode**)(t+1);
	t->bucket_cnt=bucket_cnt;
	t->manager=m;
	if(key_free_fn==NULL){
		key_free_fn=default_free_fn;
	}
	t->key_free_fn=key_free_fn;
	if(value_free_fn==NULL){
		value_free_fn=default_free_fn;
	}
	t->value_free_fn=value_free_fn;
	t->pool_for_nodes=prepare_pool(sizeof(TowerNode), n);
	return t;
}

int tower_insert(HashTower*t, 
				  char*key, void*val_data){
	int idx;
	TowerNode*p;
	HASH(key, t->bucket_cnt, &idx);
	p=t->bucket[idx];
	while(p){
		if(!is_in_set(t->manager->
						   using_lvs, p->layer_create)){
			t->key_free_fn(p->key);
			t->value_free_fn(p->data);
			p->key=key;
			t->value_free_fn(p->data);
			p->data=val_data;
			p->layer_create=t->manager->current->lv;
			return 1;
		}
		if(strcmp(key, p->key)==0){
			return 0;
		}p=p->next;
	}
	p=pool_alloc(t->pool_for_nodes);
	p->key=key;
	p->data=val_data;
	p->layer_create=t->manager->current->lv;
	p->next=t->bucket[idx];
	t->bucket[idx]=p;
    return 1;
}

void*tower_find(HashTower*t,
				const char*key){
	int idx;
	TowerNode*p;
	HASH(key, t->bucket_cnt, &idx);
	p=t->bucket[idx];
	while(p){
		if(strcmp(key, p->key)==0){
			if(!is_in_set(t->manager->
							   using_lvs, p->layer_create)){
				return NULL;
			}
			return p->data;
		}p=p->next;
	}
	return NULL;
}

int is_in_tower(HashTower*t,
				const char*key){
	int idx;
	TowerNode*p;
	HASH(key, t->bucket_cnt, &idx);
	p=t->bucket[idx];
	while(p){
		if(strcmp(key, p->key)==0){
			if(!is_in_set(t->manager->
						  using_lvs, p->layer_create)){
				return 0;
			}
			return 1;
		}p=p->next;
	}
	return 0;
}

void tower_add_layer(LayerManager*m){
	Layer*new_layer=(Layer*)
	pool_alloc(m->pool_for_layers);
	new_layer->lv=++m->layer_max;
	new_layer->prv_lv=m->current;
	m->current=new_layer;
	set_insert(m->using_lvs, new_layer->lv);
}

void tower_cut_layer(LayerManager*m){
	Layer*old_layer=m->current;
	m->current=old_layer->prv_lv;
	set_delete(m->using_lvs, old_layer->lv);
	pool_return(m->pool_for_layers, old_layer);
}

void clear_tower(HashTower*t){
    int i;
    if(!t) return;
    for(i=0;i<t->bucket_cnt;i++){
        TowerNode*p=t->bucket[i];
        while(p){
            TowerNode*next_node=p->next;
            t->key_free_fn(p->key);
            t->value_free_fn(p->data);
            p=next_node;
        }
    }
}

void destroy_tower(HashTower*t){
	destroy_pool(t->pool_for_nodes);
	free(t);
}

void destroy_layer_manager(LayerManager*m){
	Layer*l=m->current;
	while(l){
		Layer*next_layer=l->prv_lv;
		pool_return(m->pool_for_layers, l);
		l=next_layer;
	}
	destroy_set(m->using_lvs);
	destroy_pool(m->pool_for_layers);
	free(m);
}

void show_tower(HashTower*t,
				value_show_fn show_fn){
	int i;
	printf("==========HashTower==========\n");
	for(i=0; i<t->bucket_cnt; i++){
		TowerNode*p=t->bucket[i];
		while(p){
			if(is_in_set(t->manager->
						 using_lvs, p->layer_create)){
				printf("%s:",p->key);
				show_fn(p->data);
				printf("\n");
			}
			p=p->next;
		}
	}
	printf("=============================\n");
}
