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

    int insertNewState(int systemStateNr, int propertyStateNr, int parent, bool &isNewState);

    void setAcceptedState(int stateNr);

    int encode(const char *stateId);

    string getStateId(int systemStateNr, int propertyStateNr);

    string getSeqId(CounterExample *cx);

    void dump(int stateNr, bool isDotFile);

    void handleCounterexample(int stateNr, list<int> &path);

    void findAllCounterexamples();

    void findAllCounterexampleStates();

    list<CounterExample *> getAllCounterexamples() const;

    //
    // Find all counterexamples
    //
    void dfs(int v);

    //
    // SCC analysis on the fly
    //
    void pushState(int v);

    void updateLowLink(int v, int w, bool isNewState);

    bool generateSCC(int v, list<int> &path);

private:
    struct ProductState {
        ProductState(int stateNr, int systemStateNr, int propertyStateNr, int parent);

        const int parent;
        const int systemStateNr;
        const int propertyStateNr;
        int stateNr; // The state number, also the DFS traversal number
        Vector<int> nextStates;
        bool acceptedState;

        // SCC analysis by Tarjan algorithm
        int lowLink;
        bool onStack;
        int component; // The number of the component we're in
        // Find all counterexamples
        bool visited;
    };

    Vector<ProductState *> seen;
    StringTable stateTable;
    StringTable seqTable;
    NatSet visited;

    // SCC analysis
    struct ComponentInfo {
        ComponentInfo();
        bool accepted;
        list<int> circle;
    };
    stack<int> stateStack;
    Vector<ComponentInfo *> components;
    // Find all counterexamples
    NatSet acceptedStates;
    list<int> path;
    list<CounterExample *> counterexamples;
};
#endif
