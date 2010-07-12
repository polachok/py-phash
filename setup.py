from distutils.core import setup, Extension

pHashModule = Extension('pHash',
                    sources = ['phashmodule.C'],
		    language='c++',
		    libraries = ['pHash'])

setup (name = 'pHash',
       version = '0.1',
       description = 'perceptual hashing',
       ext_modules = [pHashModule])
