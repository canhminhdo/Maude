/*

    This file is part of the Maude 3 interpreter.

    Copyright 1997-2020 SRI International, Menlo Park, CA 94025, USA.

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
//      Class for symbols for built-in meta-level operations.
//
#ifndef _metaLevelOpSymbol_hh_
#define _metaLevelOpSymbol_hh_

#include "freeSymbol.hh"
#include "sequenceSearch.hh"
#include "metaModule.hh"
#include "userLevelRewritingContext.hh"

class MetaLevelOpSymbol : public FreeSymbol {
    NO_COPYING(MetaLevelOpSymbol);

public:
    //
    //	This class exists so that we can cache a variable alias
    //	map together with a corresponding parser in a MetaLevelOpCache.
    //
    class AliasMapParserPair : public CacheableState {
        AliasMapParserPair();

        ~AliasMapParserPair();

        MixfixModule::AliasMap aliasMap;
        MixfixParser *parser;

        friend class MetaLevelOpSymbol;

        friend class InterpreterManagerSymbol;
    };

    MetaLevelOpSymbol(int id, int nrArgs, const Vector<int> &strategy);

    ~MetaLevelOpSymbol();

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

    void postInterSymbolPass();

    void reset();

    bool eqRewrite(DagNode *subject, RewritingContext &context);

    MetaLevel *getMetaLevel() const;

private:
    typedef bool (MetaLevelOpSymbol::*DescentFunctionPtr)
            (FreeDagNode *subject, RewritingContext &context);

    static RewritingContext *term2RewritingContext(Term *term, RewritingContext &context);

    PreModule *getPreModule(int name);

    MatchSearchState *makeMatchSearchState(MetaModule *m,
                                           FreeDagNode *subject,
                                           RewritingContext &context) const;

    MatchSearchState *makeMatchSearchState2(MetaModule *m,
                                            FreeDagNode *subject,
                                            RewritingContext &context) const;

    RewriteSequenceSearch *makeRewriteSequenceSearch(MetaModule *m,
                                                     FreeDagNode *subject,
                                                     RewritingContext &context) const;

    SMT_RewriteSequenceSearch *makeSMT_RewriteSequenceSearch(MetaModule *m,
                                                             FreeDagNode *subject,
                                                             RewritingContext &context) const;

    NarrowingSearchState2 *makeNarrowingSearchState2(MetaModule *m,
                                                     FreeDagNode *subject,
                                                     RewritingContext &context,
                                                     int variantFlags) const;

    StrategicSearch *makeStrategicSearch(MetaModule *m,
                                         FreeDagNode *subject,
                                         RewritingContext &context,
                                         bool depthSearch) const;

    bool metaUnify2(FreeDagNode *subject,
                    RewritingContext &context,
                    bool disjoint,
                    bool irredundant);

    bool metaGetVariant2(FreeDagNode *subject, RewritingContext &context, bool irredundant);

    bool metaVariantUnify2(FreeDagNode *subject, RewritingContext &context, bool disjoint);

    bool okToBind();

    bool downSearchType(DagNode *arg, SequenceSearch::SearchType &searchType) const;

    NarrowingSequenceSearch *makeNarrowingSequenceSearch(MetaModule *m,
                                                         FreeDagNode *subject,
                                                         RewritingContext &context) const;

    NarrowingSequenceSearch *makeNarrowingSequenceSearchAlt(MetaModule *m,
                                                            FreeDagNode *subject,
                                                            RewritingContext &context) const;

    NarrowingSequenceSearch3 *makeNarrowingSequenceSearch3(MetaModule *m,
                                                           FreeDagNode *subject,
                                                           RewritingContext &context,
                                                           int variantFlags) const;

    DagNode *makeNarrowingSearchPathResult(MetaModule *m, NarrowingSequenceSearch3 *state) const;

    bool complexStrategy(DagNode *subject, RewritingContext &context);

    bool metaStratRewrite(FreeDagNode *subject, RewritingContext &context,
                          DagNode *(StrategicSearch::*func)(void));

    //
    //	Descent signature (generated by macro expansion).
    //
#define MACRO(SymbolName, NrArgs) \
  bool SymbolName(FreeDagNode* subject, RewritingContext& context);

#include "descentSignature.cc"

#undef MACRO

    bool dagifySubstitution(const Vector<Term *> &variables,
                            Vector<Term *> &values,
                            Vector<DagRoot *> &dags,
                            RewritingContext &context);

    void initializeSubstitution(Vector<Symbol *> &variables,
                                Vector<Term *> &values,
                                VariableInfo &rule,
                                Substitution &substitution);

    //
    //	Legacy stuff.
    //
    bool legacyMetaUnify2(FreeDagNode *subject, RewritingContext &context, bool disjoint);

    bool legacyMetaGetVariant2(FreeDagNode *subject, RewritingContext &context, bool irredundant);

    bool legacyMetaVariantUnify2(FreeDagNode *subject, RewritingContext &context, bool disjoint);

    DescentFunctionPtr descentFunction;
    MetaLevel *metaLevel;
    MetaLevelOpSymbol *shareWith;
};

inline MetaLevel *
MetaLevelOpSymbol::getMetaLevel() const {
    Assert(metaLevel != 0, "null metaLevel");
    return metaLevel;
}

inline
MetaLevelOpSymbol::AliasMapParserPair::AliasMapParserPair() {
    parser = 0;
}

#endif
