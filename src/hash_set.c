# include"hash.h"
# include"pool.h"

# define HASH(key, bucket_cnt, out)\
do{\
	int hash_value=0,\
	mask=bucket_cnt-1;\
	hash_value=key*2654435761U;\
	*out=hash_value&mask;\
}while(0)

typedef struct SetNode{
	int num_key;
	struct SetNode*next;
}SetNode;

struct Set{
	SetNode**bucket;
	int bucket_cnt;
	Pool*pool_for_nodes;
};

Set*create_set(int bucket_cnt){
	Set*s;
	int actual_bucket_cnt=1;
	while(actual_bucket_cnt<bucket_cnt){
		actual_bucket_cnt<<=1;
	}
	s=calloc(sizeof(Set)+
			 actual_bucket_cnt*sizeof(SetNode*), 1);
	
	s->bucket_cnt=actual_bucket_cnt;
	s->bucket=(SetNode**)(s+1);
	s->pool_for_nodes=
	prepare_pool(sizeof(SetNode), bucket_cnt);
	return s;
}

void destroy_set(Set*s){
	int i;
	if(!s){
		return;
	}
	for(i=0; i<s->bucket_cnt; i++){
		SetNode*p=s->bucket[i];
		while(p){
			SetNode*next=p->next;
			pool_return(s->pool_for_nodes, p);
			p=next;
		}
	}
	destroy_pool(s->pool_for_nodes);
	free(s);
}

void set_insert(Set*s , int key){
	int idx;
	SetNode*p;
	HASH(key, s->bucket_cnt, &idx);
	p=s->bucket[idx];
	while(p){
		if(p->num_key==key){
			return;
		}
		p=p->next;
	}
	p=pool_alloc(s->pool_for_nodes);
	p->num_key=key;
	p->next=s->bucket[idx];
	s->bucket[idx]=p;
}

int is_in_set(Set*s, int key){
	int idx;
	SetNode*p;
	HASH(key, s->bucket_cnt, &idx);
	p=s->bucket[idx];
	while(p){
		if(p->num_key==key){
			return 1;
		}
		p=p->next;
	}
	return 0;
}

void set_delete(Set*s, int key){
	int idx;
	SetNode*p, *prev=NULL;
	HASH(key, s->bucket_cnt, &idx);
	p=s->bucket[idx];
	while(p){
		if(p->num_key==key){
			if(prev){
				prev->next=p->next;
			}else{
				s->bucket[idx]=p->next;
			}
			pool_return(s->pool_for_nodes, p);
			return;
		}
		prev=p;
		p=p->next;
	}
}
