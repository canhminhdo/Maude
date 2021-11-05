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
//      Implementation for class ModelCheckerSymbolExt.
//

//      utility stuff
#include "macros.hh"
#include "vector.hh"

//      forward declarations
#include "temporal.hh"
#include "interface.hh"
#include "core.hh"
#include "freeTheory.hh"
#include "builtIn.hh"
#include "higher.hh"

//      interface class definitions
#include "symbol.hh"
#include "dagNode.hh"
#include "term.hh"

//      core class definitions
#include "dagArgumentIterator.hh"
#include "substitution.hh"
#include "rewritingContext.hh"
#include "symbolMap.hh"
#include "rule.hh"

//      free theory class definitions
#include "freeDagNode.hh"

//      built in class definitions
#include "bindingMacros.hh"

//	temporal class definitions
#include "logicFormula.hh"

//HACK
#include "quotedIdentifierSymbol.hh"
#include "quotedIdentifierDagNode.hh"

//      higher class definitions
#include "modelCheckerSymbolExt.hh"

ModelCheckerSymbolExt::ModelCheckerSymbolExt(int id) : ModelCheckerSymbol(id) {
    empModelCheckResultListSymbol = 0;
    modelCheckResultListSymbol = 0;
}

bool
ModelCheckerSymbolExt::attachData(const Vector<Sort *> &opDeclaration,
                                  const char *purpose, // name of the hook
                                  const Vector<const char *> &data) { // token name list of details of the hook
    NULL_DATA(purpose, ModelCheckerSymbolExt, data);
    return TemporalSymbol::attachData(opDeclaration, purpose, data);
}

bool
ModelCheckerSymbolExt::attachSymbol(const char *purpose, Symbol *symbol) {
    BIND_SYMBOL(purpose, symbol, empModelCheckResultListSymbol, Symbol * );
    BIND_SYMBOL(purpose, symbol, modelCheckResultListSymbol, Symbol * );
    return ModelCheckerSymbol::attachSymbol(purpose, symbol);
}

bool
ModelCheckerSymbolExt::attachTerm(const char *purpose, Term *term) {
    return ModelCheckerSymbol::attachTerm(purpose, term);
}

void
ModelCheckerSymbolExt::copyAttachments(Symbol *original, SymbolMap *map) {
    ModelCheckerSymbolExt *orig = safeCast(ModelCheckerSymbolExt * , original);
    COPY_SYMBOL(orig, satisfiesSymbol, map, Symbol * );
    COPY_SYMBOL(orig, qidSymbol, map, QuotedIdentifierSymbol * );
    COPY_SYMBOL(orig, unlabeledSymbol, map, Symbol * );
    COPY_SYMBOL(orig, deadlockSymbol, map, Symbol * );
    COPY_SYMBOL(orig, transitionSymbol, map, Symbol * );
    COPY_SYMBOL(orig, transitionListSymbol, map, Symbol * );
    COPY_SYMBOL(orig, nilTransitionListSymbol, map, Symbol * );
    COPY_SYMBOL(orig, counterexampleSymbol, map, Symbol * );
    COPY_SYMBOL(orig, empModelCheckResultListSymbol, map, Symbol * );
    COPY_SYMBOL(orig, modelCheckResultListSymbol, map, Symbol * );

    COPY_TERM(orig, trueTerm, map);
    TemporalSymbol::copyAttachments(original, map);
}

void
ModelCheckerSymbolExt::getDataAttachments(const Vector<Sort *> &opDeclaration,
                                          Vector<const char *> &purposes,
                                          Vector <Vector<const char *>> &data) {
    APPEND_DATA(purposes, data, ModelCheckerSymbolExt);
    TemporalSymbol::getDataAttachments(opDeclaration, purposes, data);
}

void
ModelCheckerSymbolExt::getSymbolAttachments(Vector<const char *> &purposes,
                                            Vector<Symbol *> &symbols) {
    APPEND_SYMBOL(purposes, symbols, empModelCheckResultListSymbol);
    APPEND_SYMBOL(purposes, symbols, modelCheckResultListSymbol);
    ModelCheckerSymbol::getSymbolAttachments(purposes, symbols);
}

void
ModelCheckerSymbolExt::getTermAttachments(Vector<const char *> &purposes,
                                          Vector<Term *> &terms) {
    ModelCheckerSymbol::getTermAttachments(purposes, terms);
}

bool
ModelCheckerSymbolExt::eqRewrite(DagNode *subject, RewritingContext &context) {
    Assert(this == subject->symbol(), "bad symbol");
    //
    //	Compute normalization of negated formula.
    //
    // casting the subject node ot a FreeDagNode
    FreeDagNode *d = safeCast(FreeDagNode * , subject);
    // Make a new context for negate of formulae, which is a new FreeDagNode
    RewritingContext *newContext = context.makeSubcontext(negate(d->getArgument(1)));
    // Reduce the new context by applying eqRewrite to the negate DagNode
    newContext->reduce();
#ifdef TDEBUG
    cout << "Negated formula: " << newContext->root() << endl;
#endif
    //
    //	Convert Dag into a LogicFormula.
    //
    SystemAutomaton system;
    LogicFormula formula;
    // create the formula with Op and add propositions in formulae to the system.propositions
    int top = build(formula, system.propositions, newContext->root());
    if (top == NONE) {
        IssueAdvisory("negated LTL formula " << QUOTE(newContext->root()) <<
                                             " did not reduce to a valid negative normal form.");
        return TemporalSymbol::eqRewrite(subject, context);
    }
    context.addInCount(*newContext); // increasing the number of equations, rewrite rules, ... for the main context
#ifdef TDEBUG
    cout << "top = " << top << endl;
    formula.dump(cout);
#endif
    //
    //	Do the model check using a ModelChecker2 object.
    //
    system.satisfiesSymbol = satisfiesSymbol;
    system.parentContext = &context;
    system.trueTerm = trueTerm.getDag();
    RewritingContext *sysContext = context.makeSubcontext(d->getArgument(0));
#ifdef TDEBUG
    cout << "Initial state: " << sysContext->root() << endl;
#endif
    system.systemStates = new StateTransitionGraph(sysContext);
    system.systemProductStates = new ProductStateTransitionGraph();
    ModelChecker2 mc(system, formula, top);
    // find all counterexamples
    bool result = mc.findCounterexamples();
//    system.systemProductStates->dump(0, true); // dump product automata
    if (result)
        system.systemProductStates->findAllCounterexamples();

    int nrSystemStates = system.systemStates->getNrStates();
    Verbose("ModelCheckerSymbol: Examined " << nrSystemStates <<
                                            " system state" << pluralize(nrSystemStates) << '.');
#ifdef TDEBUG
    if (result == true) {
        dump(*(system.systemStates), mc.getLeadIn());
        dump(*(system.systemStates), mc.getCycle());
    }
#endif
    delete newContext;
    DagNode *resultDag = result ? makeAllCounterexamples(*(system.systemStates), *(system.systemProductStates))
                                : trueTerm.getDag();

    context.addInCount(*sysContext);
    delete system.systemStates;  // deletes sysContext via ~StateTransitionGraph()
    delete system.systemProductStates;  // deletes sysContext via ~ProductStateTransitionGraph()

    return context.builtInReplace(subject, resultDag);
}

DagNode *ModelCheckerSymbolExt::makeAllCounterexamples(const StateTransitionGraph &states,
                                                       const ProductStateTransitionGraph &productStates) {
    list<ProductStateTransitionGraph::CounterExample *> counterexamples = productStates.getAllCounterexamples();
    Vector<DagNode *> cx_args;
    for(list<ProductStateTransitionGraph::CounterExample *>::iterator it = counterexamples.begin(); it != counterexamples.end(); ++it) {
        ProductStateTransitionGraph::CounterExample *cx = *it;
        Vector<DagNode *> args(2);
        int junction = cx->circle.front();
        args[0] = makeTransitionList(states, cx->path, junction);
        args[1] = makeTransitionList(states, cx->circle, junction);
        cx_args.append(counterexampleSymbol->makeDagNode(args));
    }
    if (cx_args.length() == 1)
        return cx_args[0];
    return modelCheckResultListSymbol->makeDagNode(cx_args);
}
