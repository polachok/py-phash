/*
 * Read: http://www.python.org/dev/peps/pep-0007/
 */

///--- Library needed ---///
#include <Python.h>
#include <structmember.h>
#include <pHash.h>

///--- Documentation ---///
/* The module doc string */
static char module_docstring[] =
"Python bindings for libpHash (http://phash.org/)\n\
A perceptual hash is a fingerprint of a multimedia file derived from various \n\
features from its content. Unlike cryptographic hash functions which rely on \n\
the avalanche effect of small changes in input leading to drastic changes in \n\
the output, perceptual hashes are \"close\" to one another if the features are\n\
similar.\n\
";


/* The functions doc string */
PyDoc_STRVAR( compare_images__doc__,
"compare_images(file1,file2,pcc=0.0,sigma=3.5,gamma=1.0,N=180,threshold=0.90) -> int\n\n\
Compare 2 images given the file names  \n\
Keyword arguments: \n\
file1   -- char string of first image file \n\
file2   -- char string of second image file \n\
pcc   -- (out) double value for peak of cross correlation (default 0.0)\n\
sigma   -- double value for deviation of gaussian filter (default 3.5)\n\
gamma   -- double value for gamma correction of images (default 1.0)\n\
N       -- int number for number of angles (default 180)\n\
threshold (default 0.90) \n\
return   -- int 0 (false) for different image, 1 (true) for same images, less \n\
than 0 for error \n\
");
PyDoc_STRVAR( imagehash__doc__,
"imagehash(file) -> ulong64 (aka PyLong)\n\n\
Compute dct robust image hash\n\
Keyword arguments: \n\
file     -- string variable for name of file\n\
return   -- hash of type ulong64 (aka PyLong)\n\
");
PyDoc_STRVAR( mh_imagehash__doc__,
"mh_imagehash(filename, alpha=2.0f, lvl=1.0f) -> uint8_t[]\n\n\
create MH image hash for filename image\n\
Keyword arguments: \n\
filename -- string name of image file\n\
alpha   -- int scale factor for marr wavelet (default=2)\n\
lvl   -- int level of scale factor (default = 1)\n\
return   -- uint8_t array\n\
");
PyDoc_STRVAR( image_digest__doc__,
"image_digest(file, sigma=1.0, gamma=1.0, int N=180) -> pHash.Digest\n\n\
Compute the image digest given the file name.(radial hash)\n\
Keyword arguments: \n\
file     -- string value for file name of input image.\n\
sigma   -- double value for the deviation for gaussian filter\n\
gamma   -- double value for gamma correction on the input image.\n\
N       -- int value for number of angles to consider\n\
return   -- a Digest struct\n\
");
PyDoc_STRVAR( hamming_distance__doc__,
"hamming_distance(hash1,hash2) -> int\n\n\
Compute the hamming distance between two hash (dct)\n\
Keyword arguments: \n\
hash1   -- The first hash (ulong64)\n\
hash2   -- The second hash (ulong64)\n\
return   -- The distance (over 100)\n\
");
PyDoc_STRVAR( hamming_distance2__doc__,
"hamming_distance2(hashA[], hashB[]) -> double\n\n\
Compute hamming distance between two byte arrays (Mexican Hat)\n\
Keyword arguments: \n\
hashA   -- byte array for first hash\n\
hashB   -- byte array for second hash\n\
return   -- double value for normalized hamming distance\n\
");
PyDoc_STRVAR( crosscorr__doc__,
"crosscorr(x, y, threshold=0.90)\n\n\
Compute the cross correlation of two series vectors (image_digest)\n\
Keyword arguments: \n\
x       -- Digest struct\n\
y       -- Digest struct\n\
threshold-- double value for the threshold value for which 2 images are\n\
considered the same or different.\n\
return   -- (ret,pcc)\n\
ret   -- int value - 1 (true) for same, 0 (false) for different, < 0 for error\n\
pcc   -- double value the peak of cross correlation\n\
");

///--- Globals ---///
static PyObject *pHashError;

typedef struct pHashDigest {
    PyObject_HEAD
    PyObject *id;              /* hash id */
    PyObject *coeffs;          /* the head of the digest integer coefficient array */
    int size;
} pHashDigest;

static PyTypeObject pHashDigestType = {
    PyObject_HEAD_INIT(NULL)
};

static PyMemberDef pHashDigest_members[] = {
    {"id", T_STRING, offsetof(pHashDigest, id), 0, "id"},
    {"coeffs", T_OBJECT, offsetof(pHashDigest, coeffs), 0, "coeffs"},
    {"size", T_INT, offsetof(pHashDigest, size), 0, "size"},
    {NULL}
};

///--- Foo Prototypes ---///
static PyObject * PyList_FromUint8Array(uint8_t *array, int len);
static PyObject * PyList_FromUint32Array(uint32_t *array, int len);
static PyObject * PyList_FromDoubleArray(double *array, int len);
static uint8_t* arrayFromPyList(PyObject* pyList);
static bool file_ready_for_reading (const char *filename);

static PyObject * phash_compare_images(PyObject *self, PyObject *args, PyObject *keywds);
static PyObject * phash_imagehash(PyObject *self, PyObject *args);
static PyObject * phash_mh_imagehash(PyObject *self, PyObject *args, PyObject *keywds);
static PyObject * phash_image_digest(PyObject *self, PyObject *args, PyObject *keywds);
static PyObject * phash_hamming_distance(PyObject *self, PyObject *args);
static PyObject * phash_hamming_distance2(PyObject *self, PyObject *args);
static PyObject * phash_crosscorr(PyObject *self, PyObject *args, PyObject *keywds);


///--- Definition of the pHash pythons library methods ---///
static PyMethodDef pHash_methods[] = {
    { "compare_images", (PyCFunction)phash_compare_images, METH_VARARGS|METH_KEYWORDS
        , compare_images__doc__ },
    { "imagehash", phash_imagehash, METH_VARARGS,
        imagehash__doc__ },
    { "mh_imagehash", (PyCFunction)phash_mh_imagehash, METH_VARARGS|METH_KEYWORDS,
        mh_imagehash__doc__},
    { "image_digest", (PyCFunction)phash_image_digest, METH_VARARGS|METH_KEYWORDS,
        image_digest__doc__ },
    { "hamming_distance", phash_hamming_distance, METH_VARARGS,
        hamming_distance__doc__ },
    { "hamming_distance2", phash_hamming_distance2, METH_VARARGS,
        hamming_distance2__doc__ },
    { "crosscorr", (PyCFunction)phash_crosscorr, METH_VARARGS|METH_KEYWORDS,
        crosscorr__doc__ },
    { NULL, NULL, 0, NULL}
};

///--- Useful functions for the module (not in it) --///

/* Return a new PyList from a byte array (uint8_t / unsigned char array) */
static PyObject *
PyList_FromUint8Array(uint8_t *array, int len)
{
    PyObject *pyList;
    int i;
    if (len < 0) return NULL;

    pyList = PyList_New(len);
    for (i = 0; i < len; i++) {
        PyObject *item = PyInt_FromLong((long) array[i]);
        PyList_SetItem(pyList, i, item);
    }
    return pyList;
}

/* Returns a byte (uint8_t) array from a PyList */
static uint8_t*
arrayFromPyList(PyObject* pyList)
{
    uint8_t *newarr = NULL;
    Py_ssize_t i;
    Py_ssize_t len;

    if (!pyList) return NULL;

    len = PyList_Size(pyList);
    if (len < 0) return NULL;

    newarr = (uint8_t*) malloc(len * sizeof(uint8_t));
    if (!newarr) return NULL;

    for (i = 0; i < len; i++)
        newarr[i] = (uint8_t) PyInt_AsLong(PyList_GetItem(pyList, i));

    return newarr;
}

/* return false if the file doesn't exist or does not have permission to read */
static bool
file_ready_for_reading (const char *filename)
{
    if( access( filename, R_OK ) != 0 ) {
        PyErr_SetString(pHashError,
            "The File you specified does not exist or cannot be read.");
        return false;
    }
    return true;
}

///--- Module functions ---///
/* initialization */
PyMODINIT_FUNC
initpHash(void)
{
    PyObject *m = Py_InitModule3("pHash", pHash_methods, module_docstring);
    if (m ==NULL) return;
    /* Error handler */
    pHashError = PyErr_NewException("pHash.error", NULL, NULL);
    Py_INCREF(pHashError);
    PyModule_AddObject(m, "error", pHashError);
    /* Digest type */
    pHashDigestType.tp_name   = "pHash.Digest";
    pHashDigestType.tp_basicsize = sizeof(pHashDigest);
    pHashDigestType.tp_new     = PyType_GenericNew;
    pHashDigestType.tp_methods   = NULL;
    pHashDigestType.tp_members   = pHashDigest_members;
    pHashDigestType.tp_flags     = Py_TPFLAGS_DEFAULT;
    pHashDigestType.tp_doc     = "A pHash radial digest object";
    PyType_Ready(&pHashDigestType);
    Py_INCREF(&pHashDigestType);
    PyModule_AddObject(m, "Digest", (PyObject *)&pHashDigestType);
}

static PyObject *
phash_compare_images(PyObject *self, PyObject *args, PyObject *keywds)
{
    /* set keywords and default args */
    static char *kwlist[] = {"file1", "file2", "pcc", "sigma", "gamma", "N", "threshold", NULL};
    const char *file1;
    const char *file2;
    double pcc     = 0.0;
    double sigma     = 3.5;
    double gamma     = 1.0;
    int N           = 180;
    double threshold = 0.90;
    /* take the args from the user */
    if(!PyArg_ParseTupleAndKeywords(args, keywds , "ss|dddid", kwlist,
        &file1, &file2, &pcc, &sigma, &gamma, &N, &threshold))
        return NULL;
    /* test if the files exists and ready for reading */
    if (!file_ready_for_reading(file1)) return NULL;
    if (!file_ready_for_reading(file2)) return NULL;
    /* do the comparison and return the value to the user */
    ph_compare_images(file1, file2, pcc, sigma, gamma, N, threshold);
    return PyFloat_FromDouble(pcc);
}

static PyObject *
phash_imagehash(PyObject *self, PyObject *args)
{
    const char *filename;
    ulong64 hash = 0;
    int test_imagehash;
    if(!PyArg_ParseTuple(args, "s", &filename))
        return NULL;
    /* Check if the file exist and ready for reading */
    if (!file_ready_for_reading(filename)) return NULL;

    test_imagehash = ph_dct_imagehash(filename, hash);
    /* the dct_imagehash has failed */
    if (test_imagehash == -1) {
        PyErr_SetString(pHashError,
            "The DCT robust image hash has failed.");
        return NULL;
    }
    return PyLong_FromUnsignedLongLong(hash);
}

static PyObject *
phash_mh_imagehash(PyObject *self, PyObject *args, PyObject *keywds)
{
    /* set keywords and default args */
    static char *kwlist[] = {"filename", "alpha", "lvl", NULL};
    const char *filename;
    int N        = 0;
    float alpha   = 2.0f;
    float lvl    = 1.0f;
    uint8_t* hash = 0;
    /* take the args from the user */
    if(!PyArg_ParseTupleAndKeywords(args, keywds , "s|ff", kwlist,
        &filename, &alpha, &lvl))
        return NULL;
    /* Check if the file exist and ready for reading */
    if (!file_ready_for_reading(filename)) return NULL;

    hash = ph_mh_imagehash(filename, N, alpha, lvl);
    return PyList_FromUint8Array(hash, N);
}

static PyObject *
phash_image_digest(PyObject *self, PyObject *args, PyObject *keywds)
{
    /* set keywords and default args */
    static char *kwlist[] = {"file", "sigma", "gamma", "N", NULL};
    const char *filename;
    double sigma=1.0, gamma=1.0;
    Digest dig;
    int N = 180, i;
    PyObject *coeffs, *coeff;
    pHashDigest *phdig;

    if(!PyArg_ParseTupleAndKeywords(args, keywds,"s|ddi:", kwlist,
        &filename, &sigma, &gamma, &N))
        return NULL;
    /* Check if the file exist and ready for reading */
    if (!file_ready_for_reading(filename)) return NULL;

    i = ph_image_digest(filename, sigma, gamma, dig, N);
    if (i<0) {
        PyErr_SetString(pHashError,
            "Computing the image digest of the given file has failed.");
        return NULL;
    }
    phdig = (pHashDigest *)PyObject_New(pHashDigest, &pHashDigestType);

    phdig->id = NULL; // pHash does not make use of this field for radial hashes
    coeffs = PyTuple_New(dig.size);
    for (i=0; i<dig.size; i++) {
        PyTuple_SetItem(coeffs, i, Py_BuildValue("H", dig.coeffs[i]));
    }
    phdig->coeffs = coeffs;
    phdig->size   = dig.size;
    return (PyObject *)phdig;
}

static PyObject *
phash_hamming_distance(PyObject *self, PyObject *args)
{
    ulong64 hash1, hash2;
    int ret;

    if(!PyArg_ParseTuple(args, "KK", &hash1, &hash2))
        return NULL;
    ret = ph_hamming_distance(hash1, hash2);
    return Py_BuildValue("i", ret);
}

static PyObject *
phash_hamming_distance2(PyObject *self, PyObject *args)
{
    PyObject *pyList1, *pyList2;
    uint8_t *uiarr1, *uiarr2;
    int len1, len2;
    double diff = 0.0;

    if (!PyArg_ParseTuple(args, "OO", &pyList1, &pyList2))
        return NULL;
    len1 = PyList_Size(pyList1);
    len2 = PyList_Size(pyList2);

    uiarr1 = arrayFromPyList(pyList1);
    if (!uiarr1) return NULL;
    uiarr2 = arrayFromPyList(pyList2);
    if (!uiarr2) return NULL;

    diff = ph_hammingdistance2(uiarr1, len1, uiarr2, len2);

    free(uiarr1);
    free(uiarr2);
    return PyFloat_FromDouble(diff);
}

static PyObject *
phash_crosscorr(PyObject *self, PyObject *args, PyObject *keywds)
{
    /* set keywords and default args */
    static char *kwlist[] = {"x", "y", "threshold", NULL};
    int ret, size, i;
    double pcc, threshold = 0.90;
    PyObject *py_Digest1, *py_Digest2;
    Digest digest1, digest2;
    uint8_t *coeffs1, *coeffs2;

    if(!PyArg_ParseTupleAndKeywords(args, keywds,"O!O!|d:", kwlist,
        &pHashDigestType, &py_Digest1, &pHashDigestType, &py_Digest2, &threshold))
        return NULL;

    digest1.id = NULL;
    digest2.id = NULL;

    coeffs1 = (uint8_t *)malloc(sizeof(coeffs1) * ((pHashDigest *)py_Digest1)->size);
    coeffs2 = (uint8_t *)malloc(sizeof(coeffs2) * ((pHashDigest *)py_Digest2)->size);
    if(!coeffs1 || !coeffs2){
        PyErr_SetString(pHashError,
            "One of the Digest is malformed.");
        return NULL;
    }

    ret = PyTuple_CheckExact(((pHashDigest *)py_Digest1)->coeffs);
    if(!ret) {
        PyErr_SetString(pHashError,
            "x is not a full Digest.");
        return NULL;
    }
    ret = PyTuple_CheckExact(((pHashDigest *)py_Digest2)->coeffs);
    if(!ret) {
        PyErr_SetString(pHashError,
            "y is not a full Digest.");
        return NULL;
    }

    size = ((pHashDigest *)py_Digest1)->size;
    digest1.size = size;
    digest1.coeffs = coeffs1;
    for (i=0; i<size; i++)
        digest1.coeffs[i] = (uint8_t)PyLong_AsLong(PyTuple_GetItem(((pHashDigest *)py_Digest1)->coeffs, i));

    size = ((pHashDigest *)py_Digest2)->size;
    digest2.size = size;
    digest2.coeffs = coeffs2;
    for (i=0; i<size; i++)
        digest2.coeffs[i] = (uint8_t)PyLong_AsLong(PyTuple_GetItem(((pHashDigest *)py_Digest2)->coeffs, i));

    ret = ph_crosscorr(digest1, digest2, pcc, threshold);

    free(coeffs1);
    free(coeffs2);
    return Py_BuildValue("(i,d)", ret, pcc);
}
