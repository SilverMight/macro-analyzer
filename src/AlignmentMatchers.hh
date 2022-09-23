#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <algorithm>

namespace cpp2c
{
    using namespace clang::ast_matchers;

    // Matches all AST nodes who expand to a source range that shares
    // begin/end with the given range
    AST_POLYMORPHIC_MATCHER_P2(
        alignsWithExpansion,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        cpp2c::MacroExpansionNode *, Expansion)
    {
        auto &SM = Ctx->getSourceManager();
        auto DefB = Expansion->DefinitionTokens.front().getLocation();
        auto DefE = Expansion->DefinitionTokens.back().getLocation();
        auto NodeB = SM.getSpellingLoc(Node.getBeginLoc());
        auto NodeE = SM.getSpellingLoc(Node.getEndLoc());

        // Either the node aligns with the macro itself,
        // or one of its arguments
        // TODO: Test this and make sure that its correct to include
        // the argument token ranges
        return ((NodeB == DefB) ||
                (std::any_of(Expansion->Arguments.begin(),
                             Expansion->Arguments.end(),
                             [&NodeB](MacroExpansionArgument Arg)
                             { return (!Arg.Tokens.empty()) &&
                                      (NodeB == Arg.Tokens.front()
                                                    .getLocation()); }))) &&
               (NodeE == DefE ||
                (std::any_of(Expansion->Arguments.begin(),
                             Expansion->Arguments.end(),
                             [&NodeE](MacroExpansionArgument Arg)
                             { return (!Arg.Tokens.empty()) &&
                                      (NodeE == Arg.Tokens.back()
                                                    .getLocation()); })));
    }

    // Matches all AST nodes who span the same range that the
    // given token list spans, and for whose range every token
    // in the list is spelled
    AST_POLYMORPHIC_MATCHER_P2(
        isSpelledFromTokens,
        AST_POLYMORPHIC_SUPPORTED_TYPES(clang::Decl,
                                        clang::Stmt,
                                        clang::TypeLoc),
        clang::ASTContext *, Ctx,
        std::vector<clang::Token>, Tokens)
    {
        auto &SM = Ctx->getSourceManager();
        auto B = SM.getSpellingLoc(Node.getBeginLoc());
        auto E = SM.getSpellingLoc(Node.getEndLoc());
        clang::SourceRange SpellingRange(B, E);

        // First ensure that this node spans the same range
        // as the given token list
        if (!(B == Tokens.front().getLocation() &&
              E == Tokens.back().getLocation()))
            return false;

        // Next, ensure that every token in the list is included
        // in the range spanned by this AST node
        for (auto Tok : Tokens)
            if (!SpellingRange.fullyContains(Tok.getLocation()))
                return false;

        return true;
    }
}