from cpython cimport PyObject

cdef extern from "object.h":
    ctypedef struct Object:
        pass

    void retain(Object *obj)
    void release(Object *obj)

cdef extern from "pyext.h":
    ctypedef struct OPy:
        Object proto
        PyObject *obj
    
    OPy *new_opy(object obj)

cdef extern from "hashmap.h":
    ctypedef Object *(*finder)(Node *self, int level, Object *key)
    ctypedef Node *(*inserter)(Node *self, int level, Object *key, Object *value)
    ctypedef Node *(*remover)(Node *self, int level, Object *key)

    ctypedef struct Node:
        Object proto
        finder find
        inserter insert
        remover remove

    Node *new_empty_node()

cdef class PersistentDict(object):
    cdef Node *cdict

    def __init__(self):
        self.cdict = new_empty_node()

    def __getitem__(self, key):
        ckey = <Object*>new_opy(key)
        cval = <OPy*>self.cdict.find(self.cdict, 0, ckey)
        if cval != NULL:
            return <object>cval.obj

    def setitem(self, key, value):
        ckey = <Object*>new_opy(key)
        cval = <Object*>new_opy(value)
        newdict = <Node*>self.cdict.insert(self.cdict, 0, ckey, cval)
        pydict = PersistentDict()
        pydict.cdict = newdict
        return pydict

    def delitem(self, key):
        ckey = <Object*>new_opy(key)
        newdict = <Node*>self.cdict.remove(self.cdict, 0, ckey)
        pydict = PersistentDict()
        pydict.cdict = newdict
        return pydict