#include <Python.h>
#include <pHash.h>

static PyObject *pHashError;

static PyObject *
phash_imagehash(PyObject *self, PyObject *args);

static PyMethodDef pHashMethods[] = {
    { "imagehash", phash_imagehash, METH_VARARGS,
	"Compute a DCT hash." },
    { NULL, NULL, NULL}
};

PyMODINIT_FUNC
initpHash(void) {
    PyObject *m;

    m = Py_InitModule("pHash", pHashMethods);
    if(m == NULL)
	return;

    pHashError = PyErr_NewException("pHash.error", NULL, NULL);
    Py_INCREF(pHashError);
    PyModule_AddObject(m, "error", pHashError);
}

static PyObject *
phash_imagehash(PyObject *self, PyObject *args) {
    const char *filename;
    ulong64 hash = 0;

    if(!PyArg_ParseTuple(args, "s", &filename))
	return NULL;
    ph_dct_imagehash(filename, hash);
    //return Py_BuildValue("i", hash);
    return PyLong_FromUnsignedLongLong(hash);
}
