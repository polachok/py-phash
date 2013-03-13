py-pHash
========
Python bindings for libpHash (http://phash.org/)
------------------------------------------------

A perceptual hash is a fingerprint of a multimedia file derived from various features from its content. Unlike cryptographic hash functions which rely on the avalanche effect of small changes in input leading to drastic changes in the output, perceptual hashes are "close" to one another if the features are similar.

Installation
------------

python setup.py install

(for python 3 checkout python3 branch)

Usage
-----

DCT hash
> `int phash_imagehash( str file )`
> `int phash_distance( int hash1, int hash2 )`

Radial hash
> `pHash.Digest phash_image_digest( str file, float sigma, float gamma, int angles=180 )`
> `int phash_crosscorr( phash.Digest digest1, phash.Digest digest2 )`

Mexican hat wavelet
> `list pHash.mh_imagehash( str file )`
> `float pHash.hamming_distance2( list byteList1, list byteList2 )`

Peak of Cross Correlation
> `float pHash.compare_images( str file1, str file2 )`

````python
import pHash
hash1 = pHash.imagehash( 'file.1.jpg' )
hash2 = pHash.imagehash( 'file.2.jpg' )
print 'Hamming distance: %d (%08x / %08x)' % ( pHash.hamming_distance( hash1, hash2 ), hash1, hash2 )

digest1 = pHash.image_digest( 'file.1.jpg', 1.0, 1.0, 180 )
digest2 = pHash.image_digest( 'file.2.jpg', 1.0, 1.0, 180 )
print 'Cross-correelation: %d' % ( pHash.crosscorr( digest1, digest2 ) )
````

Todo
----

- Return peak cross-correlation for radial hashing
- Add audio and video support
- Beautify the code, add comments
- Unify with python 3 branch
