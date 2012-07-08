#include <Python.h>
#include <structmember.h>
#include <pHash.h>

static PyObject *pHashError;

typedef struct pHashDigest {
	PyObject_HEAD
	PyObject *id;				//hash id
	PyObject *coeffs;           //the head of the digest integer coefficient array
    int size;
} pHashDigest;

static PyObject * phash_imagehash(PyObject *self, PyObject *args);
static PyObject * phash_image_digest(PyObject *self, PyObject *args);
static PyObject * phash_hamming_distance(PyObject *self, PyObject *args);
static PyObject * phash_crosscorr(PyObject *self, PyObject *args);

static PyMethodDef pHashMethods[] = {
    { "imagehash", phash_imagehash, METH_VARARGS,
	"Compute a DCT hash." },
    { "image_digest", phash_image_digest, METH_VARARGS,
	"Compute a radial hash." },
    { "hamming_distance", phash_hamming_distance, METH_VARARGS,
	"Compute distance." },
    { "crosscorr", phash_crosscorr, METH_VARARGS,
	"Compute radial cross correlation." },
    { NULL, NULL, NULL}
};

static PyTypeObject pHashDigestType = {
	PyObject_HEAD_INIT(NULL)
};

static PyMemberDef pHashDigest_members[] = {
		{"id", T_STRING, offsetof(pHashDigest, id), 0, "id"},
		{"coeffs", T_OBJECT, offsetof(pHashDigest, coeffs), 0, "coeffs"},
		{"size", T_INT, offsetof(pHashDigest, size), 0, "size"},
		{NULL}
	};

PyMODINIT_FUNC
initpHash(void) {
    PyObject *m;
	PyObject *coeffs;

    m = Py_InitModule("pHash", pHashMethods);
    if(m == NULL)
	return;

    pHashError = PyErr_NewException("pHash.error", NULL, NULL);
    Py_INCREF(pHashError);
    PyModule_AddObject(m, "error", pHashError);

	pHashDigestType.tp_name = "pHash.Digest";
	pHashDigestType.tp_basicsize = sizeof(pHashDigest);
	pHashDigestType.tp_new = PyType_GenericNew;
	pHashDigestType.tp_methods = { NULL };
	pHashDigestType.tp_members = pHashDigest_members;
	pHashDigestType.tp_flags = Py_TPFLAGS_DEFAULT;
	pHashDigestType.tp_doc = "A pHash radial digest object";

	PyType_Ready(&pHashDigestType);

	Py_INCREF(&pHashDigestType);
	PyModule_AddObject(m, "Digest", (PyObject *)&pHashDigestType);
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
phash_image_digest(PyObject *self, PyObject *args) {
	const char *filename;
	double sigma, gamma;
	Digest dig;
	int N = 180, i;
	PyObject *coeffs, *coeff;
	pHashDigest *phdig;

	if(!PyArg_ParseTuple(args, "sdd|i:", &filename, &sigma, &gamma, &N))
        return NULL;

	ph_image_digest(filename, sigma, gamma, dig, N);

	phdig = (pHashDigest *)PyObject_New(pHashDigest, &pHashDigestType);

	phdig->id = NULL; // pHash does not make use of this field for radial hashes

	coeffs = PyTuple_New(dig.size);
	for (i=0; i<dig.size; i++) {
		PyTuple_SetItem(coeffs, i, Py_BuildValue("H", dig.coeffs[i]));
	}

	phdig->coeffs = coeffs;
	phdig->size = dig.size;

	return (PyObject *)phdig;
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

static PyObject *
phash_crosscorr(PyObject *self, PyObject *args) {
	int ret, size, i;
	double pcc, threshold = 0.90;
	PyObject *py_Digest1, *py_Digest2;
	Digest digest1, digest2;
	uint8_t *coeffs1, *coeffs2;

	if(!PyArg_ParseTuple(args, "O!O!|d:", &pHashDigestType, &py_Digest1, &pHashDigestType, &py_Digest2, &threshold))
		return NULL;

	digest1.id = NULL;
	digest2.id = NULL;

	coeffs1 = (uint8_t *)malloc(sizeof(coeffs1) * ((pHashDigest *)py_Digest1)->size);
	coeffs2 = (uint8_t *)malloc(sizeof(coeffs2) * ((pHashDigest *)py_Digest2)->size);

	if(!coeffs1 || !coeffs2) return NULL;

	ret = PyTuple_CheckExact(((pHashDigest *)py_Digest1)->coeffs);
	if(!ret) return NULL;
	ret = PyTuple_CheckExact(((pHashDigest *)py_Digest2)->coeffs);
	if(!ret) return NULL;

	size = ((pHashDigest *)py_Digest1)->size;
	digest1.size = size;
	digest1.coeffs = coeffs1;
	for (i=0; i<size; i++) {
		digest1.coeffs[i] = (uint8_t)PyLong_AsLong(PyTuple_GetItem(((pHashDigest *)py_Digest1)->coeffs, i));
	}
	size = ((pHashDigest *)py_Digest2)->size;
	digest2.size = size;
	digest2.coeffs = coeffs2;
	for (i=0; i<size; i++) {
		digest2.coeffs[i] = (uint8_t)PyLong_AsLong(PyTuple_GetItem(((pHashDigest *)py_Digest2)->coeffs, i));
	}

	ret = ph_crosscorr(digest1, digest2, pcc, threshold);

	free(coeffs1);
	free(coeffs2);
	
	return Py_BuildValue("i", ret);
}
