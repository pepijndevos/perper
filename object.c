#include <string.h>
#include <stdlib.h>
#include "object.h"

hash_t hash(void *obj, int size) {
	hash_t hash = 0;
	int c;

	for(c = 0; c < size; c++)
		hash = *((char *) obj + c) + (hash << 6) + (hash << 16) - hash;

	return hash;
}

bool ostring_equal(Object *self, Object *other) {
	if ((self->class == other->class) && (strcmp(((OString*)self)->str, ((OString*)other)->str) == 0))
		return true;
	else
		return false;
}

ObjectType ostring_type = {ostring_equal};

OString *new_ostring(void *ctx, char *str) {
	OString *ostr = talloc(ctx, OString);
	ostr->hash = hash(str, strlen(str));
	ostr->class = &ostring_type;
	ostr->str = str;
	return ostr;
}

bool oint_equal(Object *self, Object *other) {
	if(self->class == other->class && ((OInt*)self)->n == ((OInt*)other)->n)
		return true;
	else
		return false;
}

ObjectType oint_type = {oint_equal};

OInt *new_oint(void *ctx, int n) {
	OInt *on = talloc(ctx, OInt);
	on->hash = (hash_t)n;
	on->class = &oint_type;
	on->n = n;
	return on;
}
