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

    int encode(const char *stateId);

    string getStateId(int systemStateNr, int propertyStateNr);

    string getSeqId(CounterExample *cx);

    void dump(int stateNr, bool isDotFile);

    bool sccAnalysis();

    int strongConnected(int v);

    void handleCounterexample(int stateNr);

    void findAllCounterexamples();

    list<CounterExample *> getAllCounterexamples() const;

    void dfs(int v);

private:
    struct ProductState {
        ProductState(int stateNr, int systemStateNr, int propertyStateNr, int parent);

        const int parent;
        const int systemStateNr;
        const int propertyStateNr;
        int stateNr;
        Vector<int> nextStates;
        bool acceptedState;
    };

    Vector<ProductState *> seen;
    StringTable stateTable;
    StringTable seqTable;
    NatSet visited;

    // SCC analysis
    struct StateInfo {
        int traversalNumber;	// DFS traversal number
        int component;		// number of the component we're in
        bool visited;
    };
    struct ComponentInfo {
        ComponentInfo();
        bool accepted;
        list<int> circle;
    };
    stack<int> stateStack;
    int traversalCount;
    int acceptedComponentCount;
    Vector<StateInfo> stateInfo;
    Vector<ComponentInfo *> components;
    NatSet acceptedStates;
    list<int> path;
    list<CounterExample *> counterexamples;
};
#endif
