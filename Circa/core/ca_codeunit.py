"""
Define the CodeUnit class
"""
import itertools

from term import Term

class CodeUnit(object):
    def __init__(self):
        # allTerms is a list of Terms
        self.allTerms = []

        # mainBranch is a list of Terms
        self.mainBranch = []

        # mainNamespace maps from strings to Terms
        self.mainNamespace = {}

    def _newTerm(self):
        "Internal method. Returns a new Term object."
        new_term = Term()
        self.allTerms.append(new_term)
        return new_term

    def _bootstrapEmptyTerm(self):
        """
        This function returns a new, empty term. It should only be used
        during the bootstrapping process. In other situations, use
        createTerm instead.
        """
        return self._newTerm()

    def createTerm(self, function, inputs):
        # Todo: check if we can reuse an existing term
        newTerm = self._newTerm()
        newTerm.functionTerm = function

        # Todo

        return newTerm

    def bindName(self, name, term):
        # Todo: warn if this name is already bound
        self.mainNamespace[name] = term

    def setInput(self, term, inputIndex, newInput):
        # Grow input array if necessary
        if newInput >= len(term.inputs):
            term.inputs.extend(itertools.repeat(None, inputIndex+1-len(term.inputs)))
        term.inputs[inputIndex] = newInput
        term.needsUpdate = True

    def updateAll(self):
        for term in self.allTerms:
            term.update()
