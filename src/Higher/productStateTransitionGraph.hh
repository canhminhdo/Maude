/*

    This file is part of the Maude 3 interpreter.

    Copyright 1997-2009 SRI International, Menlo Park, CA 94025, USA.

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
//	Class for building a state transition graph on-the-fly, with hash consing.
//
#ifndef _productStateTransitionGraph_hh_
#define _productStateTransitionGraph_hh_

#include <list>
#include "stringTable.hh"
#include "natSet.hh"

class ProductStateTransitionGraph {
    NO_COPYING(ProductStateTransitionGraph);

public:
    ProductStateTransitionGraph();

    ~ProductStateTransitionGraph();

    struct CounterExample {
        list<int> path;
        list<int> circle;
    };

    int getNrStates() const;

    int getNextState(int stateNr, int index);

    int getStateParent(int stateNr) const;

    int insertNewState(int systemStateNr, int propertyStateNr, int parent);

    void setAcceptedState(int stateNr);

    static int encode(const char *stateId);

    void dump(int stateNr, bool isDotFile);

    void sccAnalysis();

    int strongConnected(int v);

    void handleCounterexample(int stateNr);

    bool findAllCounterexamples();

    list<CounterExample *> getAllCounterexamples() const;

    void dfs(int v);

    int hash(int v1, int v2);

private:
    struct ProductState {
        ProductState(int systemStateNr, int propertyStateNr, int parent);

        const int parent;
        const int systemStateNr;
        const int propertyStateNr;
        int stateNr;
        Vector<int> nextStates;
        bool acceptedState;
    };

    static string getStateId(int systemStateNr, int propertyStateNr);

    Vector<ProductState *> seen;
    static StringTable stringTable;
    static NatSet visited;

    // SCC analysis
    struct StateInfo {
        int traversalNumber;	// DFS traversal number
        int component;		// number of the component we're in
    };
    struct ComponentInfo {
        ComponentInfo();
        bool accepted;
        list<int> circle;
    };
    stack<int> stateStack;
    int traversalCount;
    Vector<StateInfo> stateInfo;
    Vector<ComponentInfo *> components;
    NatSet acceptedStates;
    list<int> path;
    list<CounterExample *> counterexamples;
    NatSet seenCounterExamples; // todo: should be implemented by hash table
};

inline string
ProductStateTransitionGraph::getStateId(int systemStateNr, int propertyStateNr) {
    string stateId = to_string(systemStateNr) + "-" + to_string(propertyStateNr);
    return stateId;
}

inline int
ProductStateTransitionGraph::encode(const char *stateId) {
    int stateNr = stringTable.encode(stateId);
    return stateNr;
}

#endif
