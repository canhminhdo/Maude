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
//      Implementation for class ModelChecker2.
//

//      utility stuff
#include "macros.hh"
#include "vector.hh"

//      forward declarations
#include "temporal.hh"

//      utility class definitions
#include "modelChecker2.hh"

ModelChecker2::ModelChecker2(System &system, LogicFormula &property, int top)
        : system(system),
          propertyAutomaton(&property, top) {
    Verbose("ModelChecker: Property automaton has " <<
                                                    propertyAutomaton.getNrStates() << " states.");
}

bool
ModelChecker2::findCounterexamples() {
    intersectionStates.expandTo(1);
    const NatSet &initialStates = propertyAutomaton.getInitialStates();
    const NatSet::const_iterator e = initialStates.end();
    bool isNewState = false;
    int root = system.insertNewState(NONE, NONE, NONE, isNewState);
    acceptedComponentCount = 0;
    for (NatSet::const_iterator i = initialStates.begin(); i != e; ++i) {
        int stateNr = system.insertNewState(NONE, *i, root, isNewState);
        sccAnalysis(stateNr, NONE, *i);
    }
//    cout << "Number of accepted SCC is " << acceptedComponentCount << endl;
    return acceptedComponentCount > 0;
}

void
ModelChecker2::sccAnalysis(int stateNr, int systemStateNr, int propertyStateNr) {
    system.pushState(stateNr);
    path.push_back(stateNr);
    const BuchiAutomaton2::TransitionMap &tMap = propertyAutomaton.getTransitions(propertyStateNr);
    BuchiAutomaton2::TransitionMap::const_iterator e = tMap.end();
    for (BuchiAutomaton2::TransitionMap::const_iterator i = tMap.begin(); i != e; ++i) {
        for (int j = 0;; j++) {
            int nextSystemStateNr = getNextSystemStateNr(systemStateNr, j);
            if (nextSystemStateNr == NONE) {
                break;
            }
            if (satisfiesPropositionalFormula(nextSystemStateNr, i->second)) {
                int newPropertyStateNr = i->first;
                bool isNewState = false;
                int newStateNr = system.insertNewState(nextSystemStateNr, newPropertyStateNr, stateNr, isNewState); // todo: if the same the state -> self-loop
                if (isNewState) {
                    if (propertyAutomaton.isAccepting(newPropertyStateNr)) {
                        system.setAcceptedState(newStateNr);
                    }
                    sccAnalysis(newStateNr, nextSystemStateNr, newPropertyStateNr);
                    system.updateLowLink(stateNr, newStateNr, true);
                } else {
                    system.updateLowLink(stateNr, newStateNr, false);
                }
            }
        }
    }
    bool acceptedSCC = system.generateSCC(stateNr, path);
    path.pop_back();
    if (acceptedSCC) {
        acceptedComponentCount++;
    }
}

int
ModelChecker2::getNextSystemStateNr(int systemStateNr, int i) {
    if (systemStateNr == NONE) {
        return i == 0 ? 0 : NONE;
    }
    int n = system.getNextState(systemStateNr, i);
    if (n == NONE) {
        return NONE;
    }
    if (n >= intersectionStates.length())
        intersectionStates.expandTo(n + 1);
    return n;
}

bool
ModelChecker2::findCounterexample() {
    intersectionStates.expandTo(1);
    const NatSet &initialStates = propertyAutomaton.getInitialStates();
    const NatSet::const_iterator e = initialStates.end();
    for (NatSet::const_iterator i = initialStates.begin(); i != e; ++i) {
        if (dfs1PropertyTransitions(0, *i))
            return true;
    }
    return false;
}

//
//	First depth first search: find all reachable accepting states
//	in the intersection (synchronous product).
//

local_inline bool
ModelChecker2::dfs1SystemTransitions(int systemStateNr, int propertyStateNr) {
    intersectionStates[systemStateNr].dfs1Seen.insert(propertyStateNr);
    for (int i = 0;; i++) {
        int n = system.getNextState(systemStateNr, i);
        if (n == NONE)
            break;
        if (n >= intersectionStates.length())
            intersectionStates.expandTo(n + 1);
        if (dfs1PropertyTransitions(n, propertyStateNr))
            return true;
    }
    return false;
}

bool
ModelChecker2::dfs1PropertyTransitions(int systemStateNr, int propertyStateNr) {
    const BuchiAutomaton2::TransitionMap &tMap =
            propertyAutomaton.getTransitions(propertyStateNr);
    BuchiAutomaton2::TransitionMap::const_iterator e = tMap.end();
    for (BuchiAutomaton2::TransitionMap::const_iterator i = tMap.begin(); i != e; ++i) {
        if (satisfiesPropositionalFormula(systemStateNr, i->second)) {
            int newPropertyStateNr = i->first;
            StateSet &s = intersectionStates[systemStateNr];
            if (!(s.dfs1Seen.contains(newPropertyStateNr))) {
                s.onDfs1Stack.insert(newPropertyStateNr);
                if (dfs1SystemTransitions(systemStateNr, newPropertyStateNr) ||
                    (propertyAutomaton.isAccepting(newPropertyStateNr) &&
                     dfs2SystemTransitions(systemStateNr, newPropertyStateNr))) {
                    path.push_front(systemStateNr);
                    if (systemStateNr == cycleSystemStateNr &&
                        newPropertyStateNr == cyclePropertyStateNr)
                        cycle.swap(path);
                    return true;
                }
                intersectionStates[systemStateNr].onDfs1Stack.subtract(newPropertyStateNr);
            }
        }
    }
    return false;
}

//
//	Second depth first search: look for a path back to an intersection
//	state on the first DFS stack.
//

local_inline bool
ModelChecker2::dfs2PropertyTransitions(int systemStateNr, int propertyStateNr) {

    const BuchiAutomaton2::TransitionMap &tMap =
            propertyAutomaton.getTransitions(propertyStateNr);
    BuchiAutomaton2::TransitionMap::const_iterator e = tMap.end();
    for (BuchiAutomaton2::TransitionMap::const_iterator i = tMap.begin(); i != e; ++i) {
        if (satisfiesPropositionalFormula(systemStateNr, i->second)) {
            int newPropertyStateNr = i->first;
            StateSet &s = intersectionStates[systemStateNr];
            if (s.onDfs1Stack.contains(newPropertyStateNr)) {
                cycleSystemStateNr = systemStateNr;
                cyclePropertyStateNr = newPropertyStateNr;
                return true;
            }
            if (!(s.dfs2Seen.contains(newPropertyStateNr))) {
                if (dfs2SystemTransitions(systemStateNr, newPropertyStateNr)) {
                    path.push_front(systemStateNr);
                    return true;
                }
            }
        }
    }
    return false;
}

bool
ModelChecker2::dfs2SystemTransitions(int systemStateNr, int propertyStateNr) {
    intersectionStates[systemStateNr].dfs2Seen.insert(propertyStateNr);
    for (int i = 0;; i++) {
        int n = system.getNextState(systemStateNr, i);
        if (n == NONE)
            break;
        Assert(n < intersectionStates.length(),
               "visited system state for the first time on dfs2");
        if (dfs2PropertyTransitions(n, propertyStateNr))
            return true;
    }
    return false;
}

bool
ModelChecker2::satisfiesPropositionalFormula(int systemStateNr, Bdd formula) {
    NatSet &testedProps = intersectionStates[systemStateNr].testedProps;
    NatSet &trueProps = intersectionStates[systemStateNr].trueProps;
    for (;;) {
        if (formula == bdd_true())
            return true;
        if (formula == bdd_false())
            return false;
        int propIndex = bdd_var(formula);
        if (testedProps.contains(propIndex))
            formula = trueProps.contains(propIndex) ? bdd_high(formula) : bdd_low(formula);
        else {
            testedProps.insert(propIndex);
            if (system.checkProposition(systemStateNr, propIndex)) {
                trueProps.insert(propIndex);
                formula = bdd_high(formula);
            } else
                formula = bdd_low(formula);
        }
    }
}

