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

#ifndef RUST_AST_RESOLVE_TOPLEVEL_H
#define RUST_AST_RESOLVE_TOPLEVEL_H

#include "rust-ast-resolve-base.h"
#include "rust-ast-resolve-type.h"
#include "rust-ast-resolve-implitem.h"
#include "rust-ast-full.h"
#include "rust-name-resolver.h"

namespace Rust {
namespace Resolver {

class ResolveTopLevel : public ResolverBase
{
  using Rust::Resolver::ResolverBase::visit;

public:
  static void go (AST::Item *item, const CanonicalPath &prefix,
		  const CanonicalPath &canonical_prefix)
  {
    if (item->is_marked_for_strip ())
      return;

    ResolveTopLevel resolver (prefix, canonical_prefix);
    item->accept_vis (resolver);

    NodeId current_module = resolver.resolver->peek_current_module_scope ();
    resolver.mappings->insert_child_item_to_parent_module_mapping (
      item->get_node_id (), current_module);
  }

  void visit (AST::Module &module) override
  {
    auto mod
      = CanonicalPath::new_seg (module.get_node_id (), module.get_name ());
    auto path = prefix.append (mod);
    auto cpath = canonical_prefix.append (mod);

    resolver->get_name_scope ().insert (
      path, module.get_node_id (), module.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (module.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, mod);
    mappings->insert_module_child (current_module, module.get_node_id ());

    resolver->push_new_module_scope (module.get_node_id ());
    for (auto &item : module.get_items ())
      ResolveTopLevel::go (item.get (), path, cpath);

    resolver->pop_module_scope ();

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     module.get_node_id (), cpath);
  }

  void visit (AST::TypeAlias &alias) override
  {
    auto talias = CanonicalPath::new_seg (alias.get_node_id (),
					  alias.get_new_type_name ());
    auto path = prefix.append (talias);
    auto cpath = canonical_prefix.append (talias);

    resolver->get_type_scope ().insert (
      path, alias.get_node_id (), alias.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (alias.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, talias);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     alias.get_node_id (), cpath);
  }

  void visit (AST::TupleStruct &struct_decl) override
  {
    auto decl = CanonicalPath::new_seg (struct_decl.get_node_id (),
					struct_decl.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, struct_decl.get_node_id (), struct_decl.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (struct_decl.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     struct_decl.get_node_id (), cpath);
  }

  void visit (AST::Enum &enum_decl) override
  {
    auto decl = CanonicalPath::new_seg (enum_decl.get_node_id (),
					enum_decl.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, enum_decl.get_node_id (), enum_decl.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (enum_decl.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    for (auto &variant : enum_decl.get_variants ())
      ResolveTopLevel::go (variant.get (), path, cpath);

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     enum_decl.get_node_id (), cpath);
  }

  void visit (AST::EnumItem &item) override
  {
    auto decl
      = CanonicalPath::new_seg (item.get_node_id (), item.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, item.get_node_id (), item.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (item.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     item.get_node_id (), cpath);
  }

  void visit (AST::EnumItemTuple &item) override
  {
    auto decl
      = CanonicalPath::new_seg (item.get_node_id (), item.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, item.get_node_id (), item.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (item.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     item.get_node_id (), cpath);
  }

  void visit (AST::EnumItemStruct &item) override
  {
    auto decl
      = CanonicalPath::new_seg (item.get_node_id (), item.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, item.get_node_id (), item.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (item.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     item.get_node_id (), cpath);
  }

  void visit (AST::EnumItemDiscriminant &item) override
  {
    auto decl
      = CanonicalPath::new_seg (item.get_node_id (), item.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, item.get_node_id (), item.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (item.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    mappings->insert_canonical_path (mappings->get_current_crate (),
				     item.get_node_id (), cpath);
  }

  void visit (AST::StructStruct &struct_decl) override
  {
    auto decl = CanonicalPath::new_seg (struct_decl.get_node_id (),
					struct_decl.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, struct_decl.get_node_id (), struct_decl.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (struct_decl.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     struct_decl.get_node_id (), cpath);
  }

  void visit (AST::Union &union_decl) override
  {
    auto decl = CanonicalPath::new_seg (union_decl.get_node_id (),
					union_decl.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, union_decl.get_node_id (), union_decl.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (union_decl.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     union_decl.get_node_id (), cpath);
  }

  void visit (AST::StaticItem &var) override
  {
    auto decl
      = CanonicalPath::new_seg (var.get_node_id (), var.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_name_scope ().insert (
      path, var.get_node_id (), var.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (var.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     var.get_node_id (), cpath);
  }

  void visit (AST::ConstantItem &constant) override
  {
    auto decl = CanonicalPath::new_seg (constant.get_node_id (),
					constant.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_name_scope ().insert (
      path, constant.get_node_id (), constant.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (constant.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     constant.get_node_id (), cpath);
  }

  void visit (AST::Function &function) override
  {
    auto decl = CanonicalPath::new_seg (function.get_node_id (),
					function.get_function_name ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_name_scope ().insert (
      path, function.get_node_id (), function.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (function.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     function.get_node_id (), cpath);
  }

  void visit (AST::InherentImpl &impl_block) override
  {
    bool canonicalize_type_args = !impl_block.has_generics ();
    bool type_resolve_generic_args = false;

    CanonicalPath impl_type
      = ResolveTypeToCanonicalPath::resolve (*impl_block.get_type ().get (),
					     canonicalize_type_args,
					     type_resolve_generic_args);
    CanonicalPath impl_prefix = prefix.append (impl_type);

    for (auto &impl_item : impl_block.get_impl_items ())
      ResolveToplevelImplItem::go (impl_item.get (), impl_prefix);

    // we cannot resolve canonical paths here until later on
  }

  void visit (AST::TraitImpl &impl_block) override
  {
    bool canonicalize_type_args = !impl_block.has_generics ();
    bool type_resolve_generic_args = false;

    CanonicalPath impl_type_seg
      = ResolveTypeToCanonicalPath::resolve (*impl_block.get_type ().get (),
					     canonicalize_type_args,
					     type_resolve_generic_args);
    CanonicalPath trait_type_seg
      = ResolveTypeToCanonicalPath::resolve (impl_block.get_trait_path (),
					     canonicalize_type_args,
					     type_resolve_generic_args);

    CanonicalPath projection
      = CanonicalPath::trait_impl_projection_seg (impl_block.get_node_id (),
						  trait_type_seg,
						  impl_type_seg);
    CanonicalPath impl_prefix = prefix.append (projection);

    resolver->get_name_scope ().insert (
      impl_prefix, impl_block.get_node_id (), impl_block.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (impl_block.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    for (auto &impl_item : impl_block.get_impl_items ())
      ResolveToplevelImplItem::go (impl_item.get (), impl_prefix);

    // we cannot resolve canonical paths here until later on
  }

  void visit (AST::Trait &trait) override
  {
    auto decl
      = CanonicalPath::new_seg (trait.get_node_id (), trait.get_identifier ());
    auto path = prefix.append (decl);
    auto cpath = canonical_prefix.append (decl);

    resolver->get_type_scope ().insert (
      path, trait.get_node_id (), trait.get_locus (), false,
      [&] (const CanonicalPath &, NodeId, Location locus) -> void {
	RichLocation r (trait.get_locus ());
	r.add_range (locus);
	rust_error_at (r, "redefined multiple times");
      });

    for (auto &item : trait.get_trait_items ())
      ResolveTopLevelTraitItems::go (item.get (), path, cpath);

    NodeId current_module = resolver->peek_current_module_scope ();
    mappings->insert_module_child_item (current_module, decl);
    mappings->insert_canonical_path (mappings->get_current_crate (),
				     trait.get_node_id (), cpath);
  }

  void visit (AST::ExternBlock &extern_block) override
  {
    for (auto &item : extern_block.get_extern_items ())
      {
	ResolveToplevelExternItem::go (item.get (), prefix);
      }
  }

private:
  ResolveTopLevel (const CanonicalPath &prefix,
		   const CanonicalPath &canonical_prefix)
    : ResolverBase (), prefix (prefix), canonical_prefix (canonical_prefix)
  {}

  const CanonicalPath &prefix;
  const CanonicalPath &canonical_prefix;
};

} // namespace Resolver
} // namespace Rust

#endif // RUST_AST_RESOLVE_TOPLEVEL_H
