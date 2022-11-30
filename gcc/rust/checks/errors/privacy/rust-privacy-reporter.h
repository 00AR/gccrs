// Copyright (C) 2020-2022 Free Software Foundation, Inc.

// This file is part of GCC.

// GCC is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3, or (at your option) any later
// version.

// GCC is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with GCC; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#ifndef RUST_PRIVACY_REPORTER_H
#define RUST_PRIVACY_REPORTER_H

#include "rust-hir-map.h"
#include "rust-hir-visitor.h"
#include "rust-mapping-common.h"
#include "rust-name-resolver.h"

namespace Rust {
namespace Privacy {

class PrivacyReporter : public HIR::HIRExpressionVisitor,
			public HIR::HIRStmtVisitor
{
public:
  PrivacyReporter (Analysis::Mappings &mappings,
		   Rust::Resolver::Resolver &resolver,
		   const Rust::Resolver::TypeCheckContext &ty_ctx);

  void go (HIR::Crate &crate);

private:
  void check_for_privacy_violation (const NodeId &use_id,
				    const Location &locus);

  void check_base_type_privacy (Analysis::NodeMapping &node_mappings,
				const TyTy::BaseType *ty,
				const Location &locus);

  void check_type_privacy (const HIR::Type *type);

  virtual void visit (HIR::StructExprFieldIdentifier &field);
  virtual void visit (HIR::StructExprFieldIdentifierValue &field);
  virtual void visit (HIR::StructExprFieldIndexValue &field);

  virtual void visit (HIR::QualifiedPathInExpression &expr);
  virtual void visit (HIR::PathInExpression &expr);
  virtual void visit (HIR::ClosureExprInnerTyped &);
  virtual void visit (HIR::ClosureExprInner &expr);
  virtual void visit (HIR::StructExprStructFields &);
  virtual void visit (HIR::StructExprStruct &);
  virtual void visit (HIR::LiteralExpr &expr);
  virtual void visit (HIR::BorrowExpr &expr);
  virtual void visit (HIR::DereferenceExpr &expr);
  virtual void visit (HIR::ErrorPropagationExpr &expr);
  virtual void visit (HIR::NegationExpr &expr);
  virtual void visit (HIR::ArithmeticOrLogicalExpr &expr);
  virtual void visit (HIR::ComparisonExpr &expr);
  virtual void visit (HIR::LazyBooleanExpr &expr);
  virtual void visit (HIR::TypeCastExpr &expr);
  virtual void visit (HIR::AssignmentExpr &expr);
  virtual void visit (HIR::CompoundAssignmentExpr &expr);
  virtual void visit (HIR::GroupedExpr &expr);
  virtual void visit (HIR::ArrayExpr &expr);
  virtual void visit (HIR::ArrayIndexExpr &expr);
  virtual void visit (HIR::TupleExpr &expr);
  virtual void visit (HIR::TupleIndexExpr &expr);
  virtual void visit (HIR::CallExpr &expr);
  virtual void visit (HIR::MethodCallExpr &expr);
  virtual void visit (HIR::FieldAccessExpr &expr);
  virtual void visit (HIR::BlockExpr &expr);
  virtual void visit (HIR::ContinueExpr &expr);
  virtual void visit (HIR::BreakExpr &expr);
  virtual void visit (HIR::RangeFromToExpr &expr);
  virtual void visit (HIR::RangeFromExpr &expr);
  virtual void visit (HIR::RangeToExpr &expr);
  virtual void visit (HIR::RangeFullExpr &expr);
  virtual void visit (HIR::RangeFromToInclExpr &expr);
  virtual void visit (HIR::RangeToInclExpr &expr);
  virtual void visit (HIR::ReturnExpr &expr);
  virtual void visit (HIR::UnsafeBlockExpr &expr);
  virtual void visit (HIR::LoopExpr &expr);
  virtual void visit (HIR::WhileLoopExpr &expr);
  virtual void visit (HIR::WhileLetLoopExpr &expr);
  virtual void visit (HIR::ForLoopExpr &expr);
  virtual void visit (HIR::IfExpr &expr);
  virtual void visit (HIR::IfExprConseqElse &expr);
  virtual void visit (HIR::IfExprConseqIf &expr);
  virtual void visit (HIR::IfExprConseqIfLet &expr);
  virtual void visit (HIR::IfLetExpr &expr);
  virtual void visit (HIR::IfLetExprConseqElse &expr);
  virtual void visit (HIR::IfLetExprConseqIf &expr);
  virtual void visit (HIR::IfLetExprConseqIfLet &expr);
  virtual void visit (HIR::MatchExpr &expr);
  virtual void visit (HIR::AwaitExpr &expr);
  virtual void visit (HIR::AsyncBlockExpr &expr);

  virtual void visit (HIR::EnumItemTuple &);
  virtual void visit (HIR::EnumItemStruct &);
  virtual void visit (HIR::EnumItem &item);
  virtual void visit (HIR::TupleStruct &tuple_struct);
  virtual void visit (HIR::EnumItemDiscriminant &);
  virtual void visit (HIR::TypePathSegmentFunction &segment);
  virtual void visit (HIR::TypePath &path);
  virtual void visit (HIR::QualifiedPathInType &path);
  virtual void visit (HIR::Module &module);
  virtual void visit (HIR::ExternCrate &crate);
  virtual void visit (HIR::UseDeclaration &use_decl);
  virtual void visit (HIR::Function &function);
  virtual void visit (HIR::TypeAlias &type_alias);
  virtual void visit (HIR::StructStruct &struct_item);
  virtual void visit (HIR::Enum &enum_item);
  virtual void visit (HIR::Union &union_item);
  virtual void visit (HIR::ConstantItem &const_item);
  virtual void visit (HIR::StaticItem &static_item);
  virtual void visit (HIR::Trait &trait);
  virtual void visit (HIR::ImplBlock &impl);
  virtual void visit (HIR::ExternBlock &block);
  virtual void visit (HIR::EmptyStmt &stmt);
  virtual void visit (HIR::LetStmt &stmt);
  virtual void visit (HIR::ExprStmtWithoutBlock &stmt);
  virtual void visit (HIR::ExprStmtWithBlock &stmt);

  Analysis::Mappings &mappings;
  Rust::Resolver::Resolver &resolver;
  const Rust::Resolver::TypeCheckContext &ty_ctx;

  // `None` means we're in the root module - the crate
  Optional<NodeId> current_module;
};

} // namespace Privacy
} // namespace Rust

#endif // !RUST_PRIVACY_REPORTER_H
