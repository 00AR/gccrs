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

#ifndef RUST_AST_RESOLVE_TYPE_H
#define RUST_AST_RESOLVE_TYPE_H

#include "rust-ast-resolve-base.h"
#include "rust-ast-full.h"

namespace Rust {
namespace Resolver {

class ResolveRelativeTypePath
{
public:
  static bool go (AST::TypePath &path, NodeId &resolved_node_id);
};

class ResolveRelativeQualTypePath : public ResolverBase
{
  using ResolverBase::visit;

public:
  static bool go (AST::QualifiedPathInType &path);

  void visit (AST::TypePathSegmentGeneric &seg) override;

  void visit (AST::TypePathSegment &seg) override;

protected:
  bool resolve_qual_seg (AST::QualifiedPathType &seg);

private:
  ResolveRelativeQualTypePath ();

  bool failure_flag;
};

class ResolveType : public ResolverBase
{
  using Rust::Resolver::ResolverBase::visit;

public:
  static NodeId go (AST::Type *type,
		    bool canonicalize_type_with_generics = false,
		    CanonicalPath *canonical_path = nullptr)
  {
    ResolveType resolver (canonicalize_type_with_generics, canonical_path);
    type->accept_vis (resolver);
    return resolver.resolved_node;
  }

  static void type_resolve_generic_args (AST::GenericArgs &args)
  {
    for (auto &gt : args.get_type_args ())
      ResolveType::go (gt.get ());
  }

  void visit (AST::BareFunctionType &fntype) override
  {
    for (auto &param : fntype.get_function_params ())
      ResolveType::go (param.get_type ().get (), fntype.get_node_id ());

    if (fntype.has_return_type ())
      ResolveType::go (fntype.get_return_type ().get (), fntype.get_node_id ());
  }

  void visit (AST::TupleType &tuple) override
  {
    if (tuple.is_unit_type ())
      {
	resolved_node = resolver->get_unit_type_node_id ();
	return;
      }

    for (auto &elem : tuple.get_elems ())
      ResolveType::go (elem.get (), tuple.get_node_id ());
  }

  void visit (AST::TypePath &path) override
  {
    if (ResolveRelativeTypePath::go (path, resolved_node))
      {
	if (canonical_path == nullptr)
	  return;

	const CanonicalPath *type_path = nullptr;
	if (mappings->lookup_canonical_path (mappings->get_current_crate (),
					     resolved_node, &type_path))
	  {
	    *canonical_path = *type_path;
	  }
      }
  }

  void visit (AST::QualifiedPathInType &path) override
  {
    ResolveRelativeQualTypePath::go (path);
  }

  void visit (AST::ArrayType &type) override;

  void visit (AST::ReferenceType &type) override;

  void visit (AST::InferredType &type) override;

  void visit (AST::NeverType &type) override;

  void visit (AST::RawPointerType &type) override;

  void visit (AST::TraitObjectTypeOneBound &type) override;

  void visit (AST::TraitObjectType &type) override;

  void visit (AST::SliceType &type) override;

private:
  ResolveType (bool canonicalize_type_with_generics,
	       CanonicalPath *canonical_path)
    : ResolverBase (),
      canonicalize_type_with_generics (canonicalize_type_with_generics),
      canonical_path (canonical_path)
  {}

  bool canonicalize_type_with_generics;
  CanonicalPath *canonical_path;
};

class ResolveTypeBound : public ResolverBase
{
  using Rust::Resolver::ResolverBase::visit;

public:
  static NodeId go (AST::TypeParamBound *type,
		    bool canonicalize_type_with_generics = false)
  {
    ResolveTypeBound resolver (canonicalize_type_with_generics);
    type->accept_vis (resolver);
    return resolver.resolved_node;
  };

  void visit (AST::TraitBound &bound) override
  {
    resolved_node = ResolveType::go (&bound.get_type_path (),
				     canonicalize_type_with_generics);
  }

private:
  ResolveTypeBound (bool canonicalize_type_with_generics)
    : ResolverBase (),
      canonicalize_type_with_generics (canonicalize_type_with_generics)
  {}

  bool canonicalize_type_with_generics;
};

class ResolveGenericParam : public ResolverBase
{
  using Rust::Resolver::ResolverBase::visit;

public:
  static NodeId go (AST::GenericParam *param)
  {
    ResolveGenericParam resolver;
    param->accept_vis (resolver);
    return resolver.resolved_node;
  }

  void visit (AST::ConstGenericParam &) override
  {
    // For now do not do anything and accept everything.
    // FIXME: This needs to change soon!
  }

  void visit (AST::TypeParam &param) override
  {
    // if it has a type lets resolve it
    if (param.has_type ())
      ResolveType::go (param.get_type ().get (), param.get_node_id ());

    if (param.has_type_param_bounds ())
      {
	for (auto &bound : param.get_type_param_bounds ())
	  {
	    ResolveTypeBound::go (bound.get (), param.get_node_id ());
	  }
      }

    auto seg = CanonicalPath::new_seg (param.get_node_id (),
				       param.get_type_representation ());
    resolver->get_type_scope ().insert (
      seg, param.get_node_id (), param.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	rust_error_at (param.get_locus (),
		       "generic param redefined multiple times");
	rust_error_at (locus, "was defined here");
      });

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     param.get_node_id (), seg);
  }

private:
  ResolveGenericParam () : ResolverBase () {}
};

class ResolveWhereClause : public ResolverBase
{
  using Rust::Resolver::ResolverBase::visit;

public:
  static void Resolve (AST::WhereClause &where_clause)
  {
    ResolveWhereClause r;
    for (auto &clause : where_clause.get_items ())
      clause->accept_vis (r);
  }

  void visit (AST::TypeBoundWhereClauseItem &item) override
  {
    ResolveType::go (item.get_type ().get (), item.get_node_id ());
    if (item.has_type_param_bounds ())
      {
	for (auto &bound : item.get_type_param_bounds ())
	  {
	    ResolveTypeBound::go (bound.get (), item.get_node_id ());
	  }
      }
  }

private:
  ResolveWhereClause () : ResolverBase () {}
};

} // namespace Resolver
} // namespace Rust

#endif // RUST_AST_RESOLVE_TYPE_H
