/*

    This file is part of the Maude 3 interpreter.

    Copyright 1997-2003 SRI International, Menlo Park, CA 94025, USA.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

//
//      Class for symbols for extending built in model checking.
//
#ifndef _modelCheckerSymbolExt_hh_
#define _modelCheckerSymbolExt_hh_

#include "temporalSymbol.hh"
#include "cachedDag.hh"
#include "modelChecker2.hh"
#include "stateTransitionGraph.hh"
#include "productStateTransitionGraph.hh"
#include "modelCheckerSymbol.hh"

class ModelCheckerSymbolExt : public ModelCheckerSymbol {
    NO_COPYING(ModelCheckerSymbolExt);

public:
    ModelCheckerSymbolExt(int id);

    bool attachData(const Vector<Sort *> &opDeclaration,
                    const char *purpose,
                    const Vector<const char *> &data);

    bool attachSymbol(const char *purpose, Symbol *symbol);

    bool attachTerm(const char *purpose, Term *term);

    void copyAttachments(Symbol *original, SymbolMap *map);

    void getDataAttachments(const Vector<Sort *> &opDeclaration,
                            Vector<const char *> &purposes,
                            Vector<Vector<const char *> > &data);

    void getSymbolAttachments(Vector<const char *> &purposes,
                              Vector<Symbol *> &symbols);

    void getTermAttachments(Vector<const char *> &purposes,
                            Vector<Term *> &terms);

    bool eqRewrite(DagNode *subject, RewritingContext &context);

    DagNode *makeAllCounterexamples(const StateTransitionGraph &states, const ProductStateTransitionGraph &productStates);
private:
    //
    //	Symbols needed for returning all counterexamples.
    //
    Symbol *empModelCheckResultListSymbol;
    Symbol *modelCheckResultListSymbol;
};

#endif
