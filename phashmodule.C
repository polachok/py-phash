#include <Python.h>
#include <pHash.h>

static PyObject *pHashError;

static PyObject * phash_imagehash(PyObject *self, PyObject *args);
static PyObject * phash_hamming_distance(PyObject *self, PyObject *args);

static PyMethodDef pHashMethods[] = {
    { "imagehash", phash_imagehash, METH_VARARGS,
	"Compute a DCT hash." },
    { "hamming_distance", phash_hamming_distance, METH_VARARGS,
	"Compute distance." },
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
    return PyLong_FromUnsignedLongLong(hash);
}

static PyObject *
phash_hamming_distance(PyObject *self, PyObject *args) {
    ulong64 hash1, hash2;
    PyObject *py_hash1, *py_hash2;
    int ret;

    if(!PyArg_ParseTuple(args, "OO", &py_hash1, &py_hash2))
	    return NULL;
    hash1 = PyLong_AsUnsignedLongLong(py_hash1);
    hash2 = PyLong_AsUnsignedLongLong(py_hash2);
    ret = ph_hamming_distance(hash1, hash2);
    return Py_BuildValue("i", ret);
}
