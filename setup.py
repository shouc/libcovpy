from distutils.core import setup, Extension

module1 = Extension('cov',
                    include_dirs=['libcovpy/include'],
                    libraries=['rt'],
                    sources=['libcovpy/coverage.c'])

setup(name='libcovpy',
      version='1.0',
      description='Python binding for gaining coverage information from binaries for fuzzers.',
      author='cshou',
      author_email='scf@ieee.org',
      url='https://github.com/shouc/libcovpy',
      ext_modules=[module1])
