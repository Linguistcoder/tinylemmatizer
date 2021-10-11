# cstlempy.pyx - Python Module, this code will be translated to C by Cython.

cdef extern from "tinylemmatizer.h":
    cdef extern void startProc(const char* flexrulesFileName, int* err)
    cdef extern void stringEval(const char * s,const char ** out,int * err)
    cdef extern void endProc()
    
# Function to be called once, before the first call to lemmatize() 
def init(Str):
    cdef int Err
    startProc(bytes(Str,'iso8859-1'),&Err)

# Function to lemmatize a full form
def lemmatize(Str):
    cdef const char * Sout
    cdef int Err
    stringEval(bytes(Str,'iso8859-1'),&Sout,&Err)
    cdef bytes py_string = Sout
    return py_string.decode('UTF-8')

# Function to be called after the last call to lemmatize()
def final():
    endProc()
    
