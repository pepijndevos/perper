#include <talloc.h>
#include "hashmap.h"

int main(int argc, char *argv[]) {
	int x;
	int y;
	OInt *key;
	OInt *val;
	BasicNode *p = new_empty_node(NULL);
	BasicNode *q;
	void *pool = NULL;
	for(x=0;x<10000;x++) {
		//pool = talloc_pool(NULL, 4096);
		for(y=0;y<1000;y++) {
			key = new_oint(pool, y);
			val = new_oint(pool, x);
			q = INSERT(p, pool, key, val);
			talloc_free(key);
			talloc_free(val);
			talloc_free(p);
			p = q;
		}
		//talloc_free(pool);
	}
	talloc_free(q);
	return 0;
}
