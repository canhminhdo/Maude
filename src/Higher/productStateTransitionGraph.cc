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
//	Implementation for class StateTransitionGraph.
//

//	utility stuff
#include "macros.hh"
#include "vector.hh"

//	forward declarations
#include "interface.hh"
#include "core.hh"
#include "higher.hh"

//	core class definitions
#include "productStateTransitionGraph.hh"

StringTable ProductStateTransitionGraph::stringTable;
NatSet ProductStateTransitionGraph::visited;

ProductStateTransitionGraph::ProductStateTransitionGraph() {
}

ProductStateTransitionGraph::~ProductStateTransitionGraph() {
    int nrStates = seen.length();
    for (int i = 0; i < nrStates; i++) {
        delete seen[i];
    }
}

int
ProductStateTransitionGraph::getNextState(int stateNr, int index) {
    ProductState *n = seen[stateNr];
    int nrNextStates = n->nextStates.length();
    if (index < nrNextStates)
        return n->nextStates[index];
    return NONE;
}

ProductStateTransitionGraph::ProductState::ProductState(int systemStateNr, int propertyStateNr, int parent)
        : systemStateNr(systemStateNr),
          propertyStateNr(propertyStateNr),
          parent(parent) {
    string stateId = getStateId(systemStateNr, propertyStateNr);
    const char *id = const_cast<char *>(stateId.c_str());
    stateNr = encode(id);
    acceptedState = false;
    #ifdef TDEBUG
    cout << parent << " -> " << stateNr << " (" << systemStateNr << "," << propertyStateNr << ")" << endl;
    #endif
}

int
ProductStateTransitionGraph::getNrStates() const {
    return seen.length();
}

int
ProductStateTransitionGraph::getStateParent(int stateNr) const {
    return seen[stateNr]->parent;
}

void
ProductStateTransitionGraph::setAcceptedState(int stateNr) {
    seen[stateNr]->acceptedState = true;
    #ifdef TDEBUG
    cout << stateNr << " is accepted state" << endl;
    #endif
}

int
ProductStateTransitionGraph::insertNewState(int systemStateNr, int propertyStateNr, int parent) {
    string stateId = getStateId(systemStateNr, propertyStateNr);
    const char *id = const_cast<char *>(stateId.c_str());
    int stateNr = encode(id);
    if (stateNr >= seen.length()) {
        Assert(stateNr == seen.length(), "new state number must equal to length of vector seen");
        seen.append(new ProductState(systemStateNr, propertyStateNr, parent));
    }
    if (parent != NONE) {
        ProductState *p = seen[parent];
        p->nextStates.append(stateNr);
        #ifdef TDEBUG
        cout << "Arrow: " << parent << " -> " << stateNr << endl;
        #endif
    }
    return stateNr;
}

void ProductStateTransitionGraph::dump(int stateNr, bool isDotFile = false) {
    visited.insert(stateNr);
    if (stateNr < seen.length()) {
        ProductState *n = seen[stateNr];
        if (isDotFile)
            if (n->acceptedState)
                cout << stateNr << " [label=\"" << stateNr << "\n(" << n->systemStateNr << "," << n->propertyStateNr << ")\",shape=doublecircle,fillcolor=cyan]" << endl;
            else
                cout << stateNr << " [label=\"" << stateNr << "\n(" << n->systemStateNr << "," << n->propertyStateNr << ")\"]" << endl;

        for (int i = 0; i < n->nextStates.length(); i++) {
            if (isDotFile)
                cout << stateNr << " -> " << n->nextStates[i] << (visited.contains(n->nextStates[i]) ? " [arrowhead=normal,color=gray52,style=\"dotted\"]" : " [arrowhead=normal]") << endl;
            else
                cout << stateNr << " -> " << n->nextStates[i] << endl;
            if (!visited.contains(n->nextStates[i]))
                dump(n->nextStates[i], isDotFile);
        }
    }
    if (stateNr == 0) {
        visited.clear();
    }
}

void ProductStateTransitionGraph::sccAnalysis() {
    int nrStates = seen.length();
    if (nrStates == 0)
        return;
    stateInfo.expandTo(nrStates);
    for (int i = 0; i < nrStates; i ++)
        stateInfo[i].traversalNumber = 0;
    traversalCount = 0;
    strongConnected(0);
    cout << "Total accepted states: " << acceptedStates.size() << endl;
}

bool ProductStateTransitionGraph::findAllCounterexamples() {
    cout << "Generating all counterexamples ..." << endl;
    dfs(0);
    cout << "Total counterexamples is " << counterexamples.size() << endl;
    return counterexamples.size() > 0;
//    int count = 0;
//    for(list<CounterExample *>::iterator cx = counterexamples.begin(); cx != counterexamples.end(); ++cx) {
//        cout << "Counterexample " << ++count << ": " << endl;
//        for(list<int>::iterator it = (*cx)->path.begin(); it != (*cx)->path.end(); ++it)
//            cout << *it << endl;
//        cout << endl;
//        for(list<int>::iterator it = (*cx)->loop.begin(); it != (*cx)->loop.end(); ++it)
//            cout << *it << endl;
//        cout << endl;
//    }
}

int ProductStateTransitionGraph::strongConnected(int v) {
    // push state v on the stack and give it a traversal order number
    stateStack.push(v);
    int vLowLink = ++traversalCount;
    stateInfo[v].traversalNumber = vLowLink;
    ProductState *s = seen[v];
    for(int i = 0, n = s->nextStates.length(); i < n; i++) {
        int w = s->nextStates[i];
        int wNumber = stateInfo[w].traversalNumber;
        if (wNumber == 0) {
            // tree edge
            int wLowLink = strongConnected(w);
            if (wLowLink < vLowLink)
                vLowLink = wLowLink;
        } else if (wNumber < vLowLink) {
            // cross or back links
            vLowLink = wNumber;
        }
    }
    if (stateInfo[v].traversalNumber == vLowLink) {
        // root of a new SCC; remove SCC from stack
        int componentCount = components.length();
        int i;
        bool isAccepted = false;
        ComponentInfo *comp = new ComponentInfo();
        do {
            i = stateStack.top();
            stateStack.pop();
            stateInfo[i].traversalNumber = INT_MAX;
            stateInfo[i].component = componentCount;
            comp->circle.push_back(i);
            if (seen[i]->acceptedState)
                isAccepted = true;
        } while (i != v);
        if (isAccepted) {
            if (comp->circle.size() == 1) {
                for (int j = 0; j < seen[i]->nextStates.length(); j++) {
                    if (seen[i]->nextStates[j] == i) {
                        cout << "SCC Accepted 1" << endl;
                        comp->accepted = true;
                    }
                }
            } else {
                comp->accepted = true;
                cout << "SCC Accepted 2" << endl;
            }
            if (comp->accepted) {
                for(list<int>::iterator it = comp->circle.begin(); it != comp->circle.end(); ++it) {
                    acceptedStates.insert(*it);
                }
            }
        }
        components.append(comp);
    }
    return vLowLink;
}

void ProductStateTransitionGraph::dfs(int v) {
    if (acceptedStates.contains(v)) {
        // a counterexample found
        handleCounterexample(v);
        return;
    }
    path.push_back(v);
    for(int i = 0; i < seen[v]->nextStates.length(); i ++) {
        int w = seen[v]->nextStates[i];
        if (w != v)
            dfs(w);
    }
    path.pop_back();
}

void ProductStateTransitionGraph::handleCounterexample(int stateNr) {
    StateInfo si = stateInfo[stateNr];
    ComponentInfo *ci = components[si.component];
    // build loop
    CounterExample *cx = new CounterExample();
    list<int> prefix;
    bool flag = false;
    for(list<int>::iterator it = ci->circle.begin(); it != ci->circle.end(); ++it) {
        ProductState *s = seen[*it];
        if (*it == stateNr) {
            cx->circle.push_back(s->systemStateNr);
            flag = true;
            continue;
        }
        if (flag) {
            cx->circle.push_back(s->systemStateNr);
        } else {
            prefix.push_back(s->systemStateNr);
        }
    }
    for(list<int>::iterator it = prefix.begin(); it != prefix.end(); ++it) {
        cx->circle.push_back(*it);
    }
    cx->circle.unique(); // remove all but the first element from every consecutive group
    // the vertex base
    int base = cx->circle.front();
    for(list<int>::reverse_iterator rit = path.rbegin(); rit != path.rend(); ++rit) {
        ProductState *s = seen[*rit];
        if (s->systemStateNr != base) {
            cx->path.push_front(s->systemStateNr);
            base = s->systemStateNr;
        }
    }
    int hashValue = 0;
    for (list<int>::iterator it = cx->path.begin(); it != cx->path.end(); ++it) {
        hashValue = hash(hashValue, *it);
    }
    for (list<int>::iterator it = cx->circle.begin(); it != cx->circle.end(); ++it) {
        hashValue = hash(hashValue, *it);
    }
    if (!seenCounterExamples.contains(hashValue)) {
        counterexamples.push_back(cx);
        seenCounterExamples.insert(hashValue);
    }
}

int ProductStateTransitionGraph::hash(int v1, int v2) {
    return (v1 * v1) ^ (v1 >> 16) ^ v2;  // best function to date on empirical measurement;
}

list<ProductStateTransitionGraph::CounterExample *> ProductStateTransitionGraph::getAllCounterexamples() const {
    return counterexamples;
}

ProductStateTransitionGraph::ComponentInfo::ComponentInfo() {
    accepted = false;
}
