/* DWARF 2 Expression Evaluator.

   Copyright (C) 2001-2021 Free Software Foundation, Inc.

   Contributed by Daniel Berlin <dan@dberlin.org>.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#if !defined (DWARF2EXPR_H)
#define DWARF2EXPR_H

#include "leb128.h"
#include "gdbtypes.h"

/* Base class that describes entries found on a DWARF expression
   evaluation stack.  */

class dwarf_entry
{
protected:
  /* Not expected to be called on it's own.  */
  dwarf_entry () = default;

public:
  virtual ~dwarf_entry () = default;

  virtual std::unique_ptr<dwarf_entry> clone () const = 0;
};

using dwarf_entry_up = std::unique_ptr<dwarf_entry>;

struct dwarf2_per_objfile;

/* The expression evaluator works with a dwarf_expr_context, describing
   its current state and its callbacks.  */
struct dwarf_expr_context
{
  /* Create a new context for the expression evaluator.

     We should ever only pass in the PER_OBJFILE and the ADDR_SIZE
     information should be retrievable from there.  The PER_OBJFILE
     contains a pointer to the PER_BFD information anyway and the
     address size information must be the same for the whole BFD.  */
  dwarf_expr_context (dwarf2_per_objfile *per_objfile, int addr_size);

  virtual ~dwarf_expr_context () = default;

  /* Push ADDR onto the stack.  */
  void push_address (CORE_ADDR addr, bool in_stack_memory);

  /* Evaluate the expression at ADDR (LEN bytes long) in a given PER_CU
     and FRAME context.

     AS_LVAL defines if the returned struct value is expected to be a
     value (false) or a location description (true).

     TYPE, SUBOBJ_TYPE and SUBOBJ_OFFSET describe the expected struct
     value representation of the evaluation result.

     The ADDR_INFO property can be specified to override the range of
     memory addresses with the passed in buffer.  */
  value *evaluate (const gdb_byte *addr, size_t len, bool as_lval,
		   dwarf2_per_cu_data *per_cu, frame_info *frame,
		   const struct property_addr_info *addr_info = nullptr,
		   struct type *type = nullptr,
		   struct type *subobj_type = nullptr,
		   LONGEST subobj_offset = 0);

private:
  /* The stack of DWARF entries.  */
  std::vector<dwarf_entry_up> m_stack;

  /* Target address size in bytes.  */
  int m_addr_size = 0;

  /* The current depth of dwarf expression recursion, via DW_OP_call*,
     DW_OP_fbreg, DW_OP_push_object_address, etc., and the maximum
     depth we'll tolerate before raising an error.  */
  int m_recursion_depth = 0, m_max_recursion_depth = 0x100;

  /* We evaluate the expression in the context of this objfile.  */
  dwarf2_per_objfile *m_per_objfile;

  /* Frame information used for the evaluation.  */
  frame_info *m_frame = nullptr;

  /* Compilation unit used for the evaluation.  */
  dwarf2_per_cu_data *m_per_cu = nullptr;

  /* Property address info used for the evaluation.  */
  const struct property_addr_info *m_addr_info = nullptr;

  /* Evaluate the expression at ADDR (LEN bytes long).  */
  void eval (const gdb_byte *addr, size_t len);

  /* Return the type used for DWARF operations where the type is
     unspecified in the DWARF spec.  Only certain sizes are
     supported.  */
  struct type *address_type () const;

  /* Push ENTRY onto the stack.  */
  void push (dwarf_entry_up entry);

  /* Return true if the expression stack is empty.  */
  bool stack_empty_p () const;

  /* Pop a top element of the stack and add as a composite piece
     with an BIT_OFFSET offset and of a BIT_SIZE size.

     If the following top element of the stack is a composite
     location description, the piece will be added to it.  Otherwise
     a new composite location description will be created, pushed on
     the stack and the piece will be added to that composite.  */
  void add_piece (ULONGEST bit_size, ULONGEST bit_offset);

  /* The engine for the expression evaluator.  Using the context in
      this object, evaluate the expression between OP_PTR and
      OP_END.  */
  void execute_stack_op (const gdb_byte *op_ptr, const gdb_byte *op_end);

  /* Pop the top item off of the stack.  */
  dwarf_entry_up pop ();

  /* Retrieve the N'th item on the stack.  */
  dwarf_entry &fetch (int n);

  /* Fetch the result of the expression evaluation in a form of
     a struct value, where TYPE, SUBOBJ_TYPE and SUBOBJ_OFFSET
     describe the source level representation of that result.
     AS_LVAL defines if the fetched struct value is expected to
     be a value or a location description.  */
  value *fetch_result (struct type *type, struct type *subobj_type,
		       LONGEST subobj_offset, bool as_lval);

  /* Return the location expression for the frame base attribute, in
     START and LENGTH.  The result must be live until the current
     expression evaluation is complete.  */
  void get_frame_base (const gdb_byte **start, size_t *length);

  /* Return the base type given by the indicated DIE at DIE_CU_OFF.
     This can throw an exception if the DIE is invalid or does not
     represent a base type.  */
  struct type *get_base_type (cu_offset die_cu_off);

  /* Execute DW_AT_location expression for the DWARF expression
     subroutine in the DIE at DIE_CU_OFF in the CU.  Do not touch
     STACK while it being passed to and returned from the called DWARF
     subroutine.  */
  void dwarf_call (cu_offset die_cu_off);

  /* Push on DWARF stack an entry evaluated for DW_TAG_call_site's
     parameter matching KIND and KIND_U at the caller of specified BATON.
     If DEREF_SIZE is not -1 then use DW_AT_call_data_value instead of
     DW_AT_call_value.  */
  void push_dwarf_reg_entry_value (call_site_parameter_kind kind,
				   call_site_parameter_u kind_u,
				   int deref_size);
};

/* Return the value of register number REG (a DWARF register number),
   read as an address in a given FRAME.  */
CORE_ADDR read_addr_from_reg (frame_info *frame, int reg);

/* Check that the current operator is either at the end of an
   expression, or that it is followed by a composition operator or by
   DW_OP_GNU_uninit (which should terminate the expression).  */
void dwarf_expr_require_composition (const gdb_byte *, const gdb_byte *,
				     const char *);

/* If <BUF..BUF_END] contains DW_FORM_block* with single DW_OP_reg* return the
   DWARF register number.  Otherwise return -1.  */
int dwarf_block_to_dwarf_reg (const gdb_byte *buf, const gdb_byte *buf_end);

/* If <BUF..BUF_END] contains DW_FORM_block* with just DW_OP_breg*(0) and
   DW_OP_deref* return the DWARF register number.  Otherwise return -1.
   DEREF_SIZE_RETURN contains -1 for DW_OP_deref; otherwise it contains the
   size from DW_OP_deref_size.  */
int dwarf_block_to_dwarf_reg_deref (const gdb_byte *buf,
				    const gdb_byte *buf_end,
				    CORE_ADDR *deref_size_return);

/* If <BUF..BUF_END] contains DW_FORM_block* with single DW_OP_fbreg(X) fill
   in FB_OFFSET_RETURN with the X offset and return 1.  Otherwise return 0.  */
int dwarf_block_to_fb_offset (const gdb_byte *buf, const gdb_byte *buf_end,
			      CORE_ADDR *fb_offset_return);

/* If <BUF..BUF_END] contains DW_FORM_block* with single DW_OP_bregSP(X) fill
   in SP_OFFSET_RETURN with the X offset and return 1.  Otherwise return 0.
   The matched SP register number depends on GDBARCH.  */
int dwarf_block_to_sp_offset (struct gdbarch *gdbarch, const gdb_byte *buf,
			      const gdb_byte *buf_end,
			      CORE_ADDR *sp_offset_return);

/* Wrappers around the leb128 reader routines to simplify them for our
   purposes.  */

static inline const gdb_byte *
gdb_read_uleb128 (const gdb_byte *buf, const gdb_byte *buf_end,
		  uint64_t *r)
{
  size_t bytes_read = read_uleb128_to_uint64 (buf, buf_end, r);

  if (bytes_read == 0)
    return NULL;
  return buf + bytes_read;
}

static inline const gdb_byte *
gdb_read_sleb128 (const gdb_byte *buf, const gdb_byte *buf_end,
		  int64_t *r)
{
  size_t bytes_read = read_sleb128_to_int64 (buf, buf_end, r);

  if (bytes_read == 0)
    return NULL;
  return buf + bytes_read;
}

static inline const gdb_byte *
gdb_skip_leb128 (const gdb_byte *buf, const gdb_byte *buf_end)
{
  size_t bytes_read = skip_leb128 (buf, buf_end);

  if (bytes_read == 0)
    return NULL;
  return buf + bytes_read;
}

/* Helper to read a uleb128 value or throw an error.  */
extern const gdb_byte *safe_read_uleb128 (const gdb_byte *buf,
					  const gdb_byte *buf_end,
					  uint64_t *r);

/* Helper to read a uleb128 value or throw an error.  */
extern const gdb_byte *safe_read_sleb128 (const gdb_byte *buf,
					  const gdb_byte *buf_end,
					  int64_t *r);

/* Helper to skip a leb128 value or throw an error.  */
extern const gdb_byte *safe_skip_leb128 (const gdb_byte *buf,
					 const gdb_byte *buf_end);

#endif /* dwarf2expr.h */
