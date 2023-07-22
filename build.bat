echo "building Debug"
python_d setup.py build_ext --inplace --cython-always --debug --cython-annotate --cython-directives="linetrace=True" --define UVLOOP_DEBUG,CYTHON_TRACE,CYTHON_TRACE_NOGIL --force
echo building Release
python setup.py build_ext --inplace --cython-always --cython-annotate --cython-directives="linetrace=True" --define UVLOOP_DEBUG,CYTHON_TRACE,CYTHON_TRACE_NOGIL --force
