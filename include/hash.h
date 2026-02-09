# ifndef HASH_H
# define HASH_H

# include<stdlib.h>

typedef struct Dict Dict;
typedef struct Set Set;
typedef struct LayerManager LayerManager;
typedef struct HashTower HashTower;
typedef void(*dict_free_fn)(void*value);
typedef void(*value_show_fn)(void*value);

Dict*create_dict(
				 int bucket_cnt, 
				 dict_free_fn key_free_fn,
				 dict_free_fn value_free_fn);
void destroy_dict(Dict*d);

void dict_insert(Dict*d, char*key, void*value);
void*dict_find(Dict*d, const char*key);
int is_in_dict(Dict*d, const char*key);
void dict_delete(Dict*d, const char*key);

Set*create_set(int bucket_cnt);
void destroy_set(Set*s);

void set_insert(Set*s , int key);
int is_in_set(Set*s, int key);
void set_delete(Set*s, int key);

LayerManager*create_layer_manager(
								  int layer_cnt);
void destroy_layer_manager(
						   LayerManager*manager);

HashTower*create_tower(int bucket_cnt,
					   LayerManager*manager, 
					   dict_free_fn key_free_fn,
					   dict_free_fn value_free_fn);
void clear_tower(HashTower*old_tower);
void destroy_tower(HashTower*old_tower);

int tower_insert(
				  HashTower*tower, 
				  char*key,
				  void*value_data);
void*tower_find(
				HashTower*tower,
				const char*key);
int is_in_tower(
				HashTower*tower,
				const char*key);

void tower_add_layer(LayerManager*manager);
void tower_cut_layer(LayerManager*manager);

void show_tower(HashTower*t,
				value_show_fn show_fn);

# endif
