# png-debloat

Tool for inspecting png chunk content and optimizing png files by removing excess and irrelevant chunks and data from file.

# Usage

```bash
Usage: png-debloat [OPTIONS] INFILE 

Utility for inspecting and cleaning png chunks.
It allows to remove excess png chunks and chunks that exceeds limits defined by libpng.
All critical chunks will be left unchanged.

OPTIONS:
  -h         -  Print this message and exit
  -t         -  Parse png and print chunk info
  -c NAME    -  Chunks to delete. Multiple flags may be specified to delete multiple chunks
  -l         -  Remove LARGE chunks, i.e. chunks size of which exceeds libpng limits
  -re        -  Remove additional data after IEND (does not affects png content)
  -o OUTFILE -  Path to output file. By default processed png will be written to {INFILE}.out

```

# Build

### Binary

```bash
cmake . 
make
```

### Python extension

```bash
python setup.py build
python setup.py install
```

# Publish python ext

> Uploading done with [twine](https://pypi.org/project/twine/) 

```bash
python setup.py sdist -d dist/
pip wheel . -w dist/
twine upload [--repository-url=...] dist/*
```

# TODO

- [x]  Docs
- [x]  Refactor all logging messages and add ability to disable logs (for python build)
- [x]  Proper error handling for python module
- [x]  Add NumPy to python dependencies and move python <-> nparray conversion into python wrapper
