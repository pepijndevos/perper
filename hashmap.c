#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hashmap.h"


//TODO: implement hash and equal for nested maps

Object *empty_find(BasicNode *self, int level, Object *key);
Object *single_find(BasicNode *self, int level, Object *key);
Object *bitmap_find(BasicNode *self, int level, Object *key);
Object *collision_find(BasicNode *self, int level, Object *key);

BasicNode *empty_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value);
BasicNode *single_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value);
BasicNode *bitmap_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value);
BasicNode *collision_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value);

BasicNode *empty_remove(BasicNode *self, void *ctx, int level, Object *key);
BasicNode *single_remove(BasicNode *self, void *ctx, int level, Object *key);
BasicNode *bitmap_remove(BasicNode *self, void *ctx, int level, Object *key);
BasicNode *collision_remove(BasicNode *self, void *ctx, int level, Object *key);

NodeType empty_type = {NULL, empty_find, empty_insert, empty_remove};
NodeType single_type = {NULL, single_find, single_insert, single_remove};
NodeType bitmap_type = {NULL, bitmap_find, bitmap_insert, bitmap_remove};
NodeType collision_type = {NULL, collision_find, collision_insert, collision_remove};

BasicNode *new_empty_node(void *ctx) {
	static BasicNode *n = NULL;
	if(n == NULL) {
		n = talloc(NULL, BasicNode);
		n->class = &empty_type;
	}
	return talloc_reference(ctx, n);
}

SingleNode *new_single_node(void *ctx) {
	SingleNode *n = talloc(ctx, SingleNode);
	n->class = &single_type;
	return n;
}

BitmapNode *new_bitmap_node(void *ctx) {
	BitmapNode *n = talloc(ctx, BitmapNode);
	n->class = &bitmap_type;
	int i;
	for(i=0;i<32;i++) {
		n->children[i] = new_empty_node(n);
	}
	return n;
}

CollisionNode *new_collision_node(void *ctx) {
	CollisionNode *n = talloc(ctx, CollisionNode);
	n->class = &collision_type;
	n->next = NULL;
	return n;
}

Object *empty_find(BasicNode *self, int level, Object *key) {
	return NULL;
}

Object *single_find(BasicNode *self, int level, Object *key) {
	if(((SingleNode*)self)->key->hash == key->hash) {
		return ((SingleNode*)self)->value;
	} else {
		return NULL;
	}
}

Object *bitmap_find(BasicNode *self, int level, Object *key) {
	BitmapNode *n = (BitmapNode*)self;
	BasicNode *child = n->children[(key->hash >> (5 * level)) & 31];
	return child->class->find(child, (level+1), key);
}

Object *collision_find(BasicNode *self, int level, Object *key) {
	SingleNode *n = (SingleNode*)self;
	CollisionNode *m = (CollisionNode*)self;
	if(n->key->class->equal(n->key, key)) {
		return n->value;
	} else if(m->next != NULL) {
		return collision_find((BasicNode*)m->next, level, key);
	} else {
		return NULL;
	}
}

BasicNode *empty_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value) {
	SingleNode *n = new_single_node(ctx);
	
	n->key = talloc_reference(n, key);
	n->value = talloc_reference(n, value);
	return (BasicNode*)n;
}

BasicNode *single_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value) {
	SingleNode *node = (SingleNode*)self;

	if(node->key->hash != key->hash) {
		BitmapNode *parent = new_bitmap_node(ctx);

		parent->children[(node->key->hash >> (5 * level)) & 31] = (BasicNode*)talloc_reference(parent, node);

		BasicNode *second_child = parent->children[(key->hash >> (5 * level)) & 31];
		parent->children[(key->hash >> (5 * level)) & 31] = second_child->class->insert(second_child, parent, (level+1), key, value);
		return (BasicNode*)parent;
	} else if (node->key->class->equal(node->key, key)) {
		SingleNode *n = new_single_node(ctx);
		
		n->key = talloc_reference(n, key);
		n->value = talloc_reference(n, value);
		return (BasicNode*)n;
	} else {
		CollisionNode *col_node = new_collision_node(ctx);
		CollisionNode *next_col_node = new_collision_node(col_node);

		((SingleNode*)col_node)->key = node->key;
		((SingleNode*)col_node)->value = node->value;
		col_node->next = next_col_node;

		((SingleNode*)next_col_node)->key = key;
		((SingleNode*)next_col_node)->value = value;
		
		return (BasicNode*)col_node;
	}
}

BasicNode *bitmap_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value) {
	BitmapNode *node = (BitmapNode*)self;
	BitmapNode *new = talloc(ctx, BitmapNode);
	memcpy(new, node, sizeof(BitmapNode)); //TODO test speed
	
	int i;
	for(i=0;i<32;i++) {
		new->children[i] = talloc_reference(new, node->children[i]);
	}

	BasicNode *child = node->children[(key->hash >> (5 * level)) & 31];
	new->children[(key->hash >> (5 * level)) & 31] = child->class->insert(child, new, (level+1), key, value);
	
	// we overwrote the incremented one with a new one.
	// reset the old one, we don't ref it.
	talloc_free(child);

	return (BasicNode*)new;
}

BasicNode *collision_insert(BasicNode *self, void *ctx, int level, Object *key, Object *value) {
	CollisionNode *n = new_collision_node(ctx);
	
	((SingleNode*)n)->key = talloc_reference(n, key);
	((SingleNode*)n)->value = talloc_reference(n, value);
	
	n->next = (CollisionNode*)talloc_reference(n, self);
	return (BasicNode*)n;
}

BasicNode *empty_remove(BasicNode *self, void *ctx, int level, Object *key) {
	//retain((Object*)self);
	return self;
}

BasicNode *single_remove(BasicNode *self, void *ctx, int level, Object *key) {
	if(((SingleNode*)self)->key->hash == key->hash) {
		return new_empty_node(ctx);
	} else {
		return talloc_reference(ctx, self);
	}
}

BasicNode *bitmap_remove(BasicNode *self, void *ctx, int level, Object *key) {
	BitmapNode *node = (BitmapNode*)self;
	BitmapNode *new = talloc(ctx, BitmapNode);
	memcpy(new, node, sizeof(BitmapNode));

	int i;
	for(i=0;i<32;i++) {
		new->children[i] = talloc_reference(new, node->children[i]);
	}
	
	BasicNode *child = new->children[(key->hash >> (5 * level)) & 31];
	new->children[(key->hash >> (5 * level)) & 31] = child->class->remove(child, new, (level+1), key);

	talloc_free(child);

	return (BasicNode*)new;
}

BasicNode *collision_remove(BasicNode *self, void *ctx, int level, Object *key) {
	SingleNode *n = (SingleNode*)self;
	CollisionNode *m = (CollisionNode*)self;
	if(n->key->class->equal(n->key, key)) {
		if (m->next != NULL) {
			return (BasicNode*)talloc_reference(ctx, m->next);
		} else {
			return NULL;
		}
	} else {
		CollisionNode *new = talloc(ctx, CollisionNode);
		memcpy(new, m, sizeof(CollisionNode));
		new->next = (CollisionNode*)collision_remove((BasicNode*)m->next, new, level, key);
		return (BasicNode*)new;
	}
}
