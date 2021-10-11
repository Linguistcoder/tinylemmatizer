# launch.py - Python 3 stub loader, loads the 'cstlempy' module that was made by Cython.

# This code is always interpreted, like normal Python.
# It is not compiled to C.

import cstlempy

filename = "flexrules"
print("Initialise cstlempy with the call 'cstlempy.init("+filename+")'.\n")
cstlempy.init(filename)

cstlempyFullForm = "went"
print("Going to call lemmatize(<full form>).\n\nThe full form is this one: "+cstlempyFullForm)
answer = cstlempy.lemmatize(cstlempyFullForm)
print('Answer from cstlempy:      '+answer+'\n')
cstlempy.final()
print("We have called cstlempy.final(), so now cstlempy is not available until we call\n'cstlempy.init()' again.")

