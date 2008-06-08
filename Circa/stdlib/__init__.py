
# Contains standard Circa symbols
#
# Includes:
#   Basic math functions
#   Debugging functions

import boolean, comparison, simple_math, map

def createFunctions(codeunit):
    boolean.createFunctions(codeunit)
    comparison.createFunctions(codeunit)
    #feedback.createFunctions(codeunit)
    map.createFunctions(codeunit)
    simple_math.createFunctions(codeunit)

