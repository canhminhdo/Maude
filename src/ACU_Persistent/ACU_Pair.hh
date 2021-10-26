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
//	Class for argument-multiplicity pairs.
//
#ifndef _ACU_Pair_hh_
#define _ACU_Pair_hh_

class ACU_Pair {
public:
    ACU_Pair();

    ACU_Pair(DagNode *d, int m);

    void set(DagNode *d, int m);

    DagNode *getDagNode() const;

    int getMultiplicity() const;

    /* HACK
  private:
    */
    DagNode *dagNode;
    int multiplicity;
};

inline
ACU_Pair::ACU_Pair() {
}

inline
ACU_Pair::ACU_Pair(DagNode *d, int m) {
    dagNode = d;
    multiplicity = m;
}

inline void
ACU_Pair::set(DagNode *d, int m) {
    dagNode = d;
    multiplicity = m;
}

inline DagNode *
ACU_Pair::getDagNode() const {
    return dagNode;
}

inline int
ACU_Pair::getMultiplicity() const {
    return multiplicity;
}

#endif
