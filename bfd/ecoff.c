/* Generic ECOFF (Extended-COFF) routines.
   Copyright 1990, 1991, 1992, 1993 Free Software Foundation, Inc.
   Original version by Per Bothner.
   Full support added by Ian Lance Taylor, ian@cygnus.com.

This file is part of BFD, the Binary File Descriptor library.

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
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "bfdlink.h"
#include "libbfd.h"
#include "aout/ar.h"
#include "aout/ranlib.h"

/* FIXME: We need the definitions of N_SET[ADTB], but aout64.h defines
   some other stuff which we don't want and which conflicts with stuff
   we do want.  */
#include "libaout.h"
#include "aout/aout64.h"
#undef N_ABS
#undef exec_hdr
#undef obj_sym_filepos

#include "coff/internal.h"
#include "coff/sym.h"
#include "coff/symconst.h"
#include "coff/ecoff.h"
#include "libcoff.h"
#include "libecoff.h"

/* Prototypes for static functions.  */

static int ecoff_get_magic PARAMS ((bfd *abfd));
static void ecoff_set_symbol_info PARAMS ((bfd *abfd, SYMR *ecoff_sym,
					   asymbol *asym, int ext,
					   asymbol **indirect_ptr_ptr));
static void ecoff_emit_aggregate PARAMS ((bfd *abfd, char *string,
					  RNDXR *rndx, long isym,
					  CONST char *which));
static char *ecoff_type_to_string PARAMS ((bfd *abfd, union aux_ext *aux_ptr,
					   unsigned int indx, int bigendian));
static boolean ecoff_slurp_reloc_table PARAMS ((bfd *abfd, asection *section,
						asymbol **symbols));
static void ecoff_compute_section_file_positions PARAMS ((bfd *abfd));
static boolean ecoff_get_extr PARAMS ((asymbol *, EXTR *));
static void ecoff_set_index PARAMS ((asymbol *, bfd_size_type));
static unsigned int ecoff_armap_hash PARAMS ((CONST char *s,
					      unsigned int *rehash,
					      unsigned int size,
					      unsigned int hlog));

/* This stuff is somewhat copied from coffcode.h.  */

static asection bfd_debug_section = { "*DEBUG*" };

/* Create an ECOFF object.  */

boolean
ecoff_mkobject (abfd)
     bfd *abfd;
{
  abfd->tdata.ecoff_obj_data = ((struct ecoff_tdata *)
				bfd_zalloc (abfd, sizeof (ecoff_data_type)));
  if (abfd->tdata.ecoff_obj_data == NULL)
    {
      bfd_error = no_memory;
      return false;
    }

  return true;
}

/* This is a hook called by coff_real_object_p to create any backend
   specific information.  */

PTR
ecoff_mkobject_hook (abfd, filehdr, aouthdr)
     bfd *abfd;
     PTR filehdr;
     PTR aouthdr;
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  struct internal_aouthdr *internal_a = (struct internal_aouthdr *) aouthdr;
  ecoff_data_type *ecoff;
  asection *regsec;

  if (ecoff_mkobject (abfd) == false)
    return NULL;

  ecoff = ecoff_data (abfd);
  ecoff->gp_size = 8;
  ecoff->sym_filepos = internal_f->f_symptr;

  /* Create the .reginfo section to give programs outside BFD a way to
     see the information stored in the a.out header.  See the comment
     in coff/ecoff.h.  */
  regsec = bfd_make_section (abfd, REGINFO);
  if (regsec == NULL)
    return NULL;
  /* Tell the linker to leave this section completely alone.  */
  regsec->flags = SEC_SHARED_LIBRARY;

  if (internal_a != (struct internal_aouthdr *) NULL)
    {
      int i;

      ecoff->text_start = internal_a->text_start;
      ecoff->text_end = internal_a->text_start + internal_a->tsize;
      ecoff->gp = internal_a->gp_value;
      ecoff->gprmask = internal_a->gprmask;
      for (i = 0; i < 4; i++)
	ecoff->cprmask[i] = internal_a->cprmask[i];
      ecoff->fprmask = internal_a->fprmask;
      if (internal_a->magic == ECOFF_AOUT_ZMAGIC)
	abfd->flags |= D_PAGED;
    }

  /* It turns out that no special action is required by the MIPS or
     Alpha ECOFF backends.  They have different information in the
     a.out header, but we just copy it all (e.g., gprmask, cprmask and
     fprmask) and let the swapping routines ensure that only relevant
     information is written out.  */

  return (PTR) ecoff;
}

/* This is a hook needed by SCO COFF, but we have nothing to do.  */

/*ARGSUSED*/
asection *
ecoff_make_section_hook (abfd, name)
     bfd *abfd;
     char *name;
{
  return (asection *) NULL;
}

/* Initialize a new section.  */

boolean
ecoff_new_section_hook (abfd, section)
     bfd *abfd;
     asection *section;
{
  section->alignment_power = abfd->xvec->align_power_min;

  if (strcmp (section->name, _TEXT) == 0)
    section->flags |= SEC_CODE | SEC_LOAD | SEC_ALLOC;
  else if (strcmp (section->name, _DATA) == 0
	   || strcmp (section->name, _SDATA) == 0)
    section->flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC;
  else if (strcmp (section->name, _RDATA) == 0
	   || strcmp (section->name, _LIT8) == 0
	   || strcmp (section->name, _LIT4) == 0)
    section->flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC | SEC_READONLY;
  else if (strcmp (section->name, _BSS) == 0
	   || strcmp (section->name, _SBSS) == 0)
    section->flags |= SEC_ALLOC;
  else if (strcmp (section->name, REGINFO) == 0)
    {
      section->flags |= SEC_HAS_CONTENTS | SEC_NEVER_LOAD;
      section->_raw_size = sizeof (struct ecoff_reginfo);
    }

  /* Probably any other section name is SEC_NEVER_LOAD, but I'm
     uncertain about .init on some systems and I don't know how shared
     libraries work.  */

  return true;
}

/* Determine the machine architecture and type.  This is called from
   the generic COFF routines.  It is the inverse of ecoff_get_magic,
   below.  This could be an ECOFF backend routine, with one version
   for each target, but there aren't all that many ECOFF targets.  */

boolean
ecoff_set_arch_mach_hook (abfd, filehdr)
     bfd *abfd;
     PTR filehdr;
{
  struct internal_filehdr *internal_f = (struct internal_filehdr *) filehdr;
  enum bfd_architecture arch;
  unsigned long mach;

  switch (internal_f->f_magic)
    {
    case MIPS_MAGIC_1:
    case MIPS_MAGIC_LITTLE:
    case MIPS_MAGIC_BIG:
      arch = bfd_arch_mips;
      mach = 3000;
      break;

    case MIPS_MAGIC_LITTLE2:
    case MIPS_MAGIC_BIG2:
      /* MIPS ISA level 2: the r6000 */
      arch = bfd_arch_mips;
      mach = 6000;
      break;

    case MIPS_MAGIC_LITTLE3:
    case MIPS_MAGIC_BIG3:
      /* MIPS ISA level 3: the r4000 */
      arch = bfd_arch_mips;
      mach = 4000;
      break;

    case ALPHA_MAGIC:
      arch = bfd_arch_alpha;
      mach = 0;
      break;

    default:
      arch = bfd_arch_obscure;
      mach = 0;
      break;
    }

  return bfd_default_set_arch_mach (abfd, arch, mach);
}

/* Get the magic number to use based on the architecture and machine.
   This is the inverse of ecoff_set_arch_mach_hook, above.  */

static int
ecoff_get_magic (abfd)
     bfd *abfd;
{
  int big, little;

  switch (bfd_get_arch (abfd))
    {
    case bfd_arch_mips:
      switch (bfd_get_mach (abfd))
	{
	default:
	case 0:
	case 3000:
	  big = MIPS_MAGIC_BIG;
	  little = MIPS_MAGIC_LITTLE;
	  break;

	case 6000:
	  big = MIPS_MAGIC_BIG2;
	  little = MIPS_MAGIC_LITTLE2;
	  break;

	case 4000:
	  big = MIPS_MAGIC_BIG3;
	  little = MIPS_MAGIC_LITTLE3;
	  break;
	}

      return abfd->xvec->byteorder_big_p ? big : little;

    case bfd_arch_alpha:
      return ALPHA_MAGIC;

    default:
      abort ();
      return 0;
    }
}

/* Get the section s_flags to use for a section.  */

long
ecoff_sec_to_styp_flags (name, flags)
     CONST char *name;
     flagword flags;
{
  long styp;

  styp = 0;

  if (strcmp (name, _TEXT) == 0)
    styp = STYP_TEXT;
  else if (strcmp (name, _DATA) == 0)
    styp = STYP_DATA;
  else if (strcmp (name, _SDATA) == 0)
    styp = STYP_SDATA;
  else if (strcmp (name, _RDATA) == 0)
    styp = STYP_RDATA;
  else if (strcmp (name, _LITA) == 0)
    styp = STYP_LITA;
  else if (strcmp (name, _LIT8) == 0)
    styp = STYP_LIT8;
  else if (strcmp (name, _LIT4) == 0)
    styp = STYP_LIT4;
  else if (strcmp (name, _BSS) == 0)
    styp = STYP_BSS;
  else if (strcmp (name, _SBSS) == 0)
    styp = STYP_SBSS;
  else if (strcmp (name, _INIT) == 0)
    styp = STYP_ECOFF_INIT;
  else if (strcmp (name, _FINI) == 0)
    styp = STYP_ECOFF_FINI;
  else if (flags & SEC_CODE) 
    styp = STYP_TEXT;
  else if (flags & SEC_DATA) 
    styp = STYP_DATA;
  else if (flags & SEC_READONLY)
    styp = STYP_RDATA;
  else if (flags & SEC_LOAD)
    styp = STYP_REG;
  else
    styp = STYP_BSS;

  if (flags & SEC_NEVER_LOAD)
    styp |= STYP_NOLOAD;

  return styp;
}

/* Get the BFD flags to use for a section.  */

/*ARGSUSED*/
flagword
ecoff_styp_to_sec_flags (abfd, hdr)
     bfd *abfd;
     PTR hdr;
{
  struct internal_scnhdr *internal_s = (struct internal_scnhdr *) hdr;
  long styp_flags = internal_s->s_flags;
  flagword sec_flags=0;

  if (styp_flags & STYP_NOLOAD)
    sec_flags |= SEC_NEVER_LOAD;

  /* For 386 COFF, at least, an unloadable text or data section is
     actually a shared library section.  */
  if ((styp_flags & STYP_TEXT)
      || (styp_flags & STYP_ECOFF_INIT)
      || (styp_flags & STYP_ECOFF_FINI))
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_CODE | SEC_SHARED_LIBRARY;
      else
	sec_flags |= SEC_CODE | SEC_LOAD | SEC_ALLOC;
    }
  else if ((styp_flags & STYP_DATA)
	   || (styp_flags & STYP_RDATA)
	   || (styp_flags & STYP_SDATA))
    {
      if (sec_flags & SEC_NEVER_LOAD)
	sec_flags |= SEC_DATA | SEC_SHARED_LIBRARY;
      else
	sec_flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC;
      if (styp_flags & STYP_RDATA)
	sec_flags |= SEC_READONLY;
    }
  else if ((styp_flags & STYP_BSS)
	   || (styp_flags & STYP_SBSS))
    {
      sec_flags |= SEC_ALLOC;
    }
  else if (styp_flags & STYP_INFO) 
    {
      sec_flags |= SEC_NEVER_LOAD;
    }
  else if ((styp_flags & STYP_LITA)
	   || (styp_flags & STYP_LIT8)
	   || (styp_flags & STYP_LIT4))
    {
      sec_flags |= SEC_DATA | SEC_LOAD | SEC_ALLOC | SEC_READONLY;
    }
  else
    {
      sec_flags |= SEC_ALLOC | SEC_LOAD;
    }

  return sec_flags;
}

/* Routines to swap auxiliary information in and out.  I am assuming
   that the auxiliary information format is always going to be target
   independent.  */

/* Swap in a type information record.
   BIGEND says whether AUX symbols are big-endian or little-endian; this
   info comes from the file header record (fh-fBigendian).  */

void
ecoff_swap_tir_in (bigend, ext_copy, intern)
     int bigend;
     struct tir_ext *ext_copy;
     TIR *intern;
{
  struct tir_ext ext[1];

  *ext = *ext_copy;		/* Make it reasonable to do in-place.  */
  
  /* now the fun stuff... */
  if (bigend) {
    intern->fBitfield   = 0 != (ext->t_bits1[0] & TIR_BITS1_FBITFIELD_BIG);
    intern->continued   = 0 != (ext->t_bits1[0] & TIR_BITS1_CONTINUED_BIG);
    intern->bt          = (ext->t_bits1[0] & TIR_BITS1_BT_BIG)
			>>		    TIR_BITS1_BT_SH_BIG;
    intern->tq4         = (ext->t_tq45[0] & TIR_BITS_TQ4_BIG)
			>>		    TIR_BITS_TQ4_SH_BIG;
    intern->tq5         = (ext->t_tq45[0] & TIR_BITS_TQ5_BIG)
			>>		    TIR_BITS_TQ5_SH_BIG;
    intern->tq0         = (ext->t_tq01[0] & TIR_BITS_TQ0_BIG)
			>>		    TIR_BITS_TQ0_SH_BIG;
    intern->tq1         = (ext->t_tq01[0] & TIR_BITS_TQ1_BIG)
			>>		    TIR_BITS_TQ1_SH_BIG;
    intern->tq2         = (ext->t_tq23[0] & TIR_BITS_TQ2_BIG)
			>>		    TIR_BITS_TQ2_SH_BIG;
    intern->tq3         = (ext->t_tq23[0] & TIR_BITS_TQ3_BIG)
			>>		    TIR_BITS_TQ3_SH_BIG;
  } else {
    intern->fBitfield   = 0 != (ext->t_bits1[0] & TIR_BITS1_FBITFIELD_LITTLE);
    intern->continued   = 0 != (ext->t_bits1[0] & TIR_BITS1_CONTINUED_LITTLE);
    intern->bt          = (ext->t_bits1[0] & TIR_BITS1_BT_LITTLE)
			>>		    TIR_BITS1_BT_SH_LITTLE;
    intern->tq4         = (ext->t_tq45[0] & TIR_BITS_TQ4_LITTLE)
			>>		    TIR_BITS_TQ4_SH_LITTLE;
    intern->tq5         = (ext->t_tq45[0] & TIR_BITS_TQ5_LITTLE)
			>>		    TIR_BITS_TQ5_SH_LITTLE;
    intern->tq0         = (ext->t_tq01[0] & TIR_BITS_TQ0_LITTLE)
			>>		    TIR_BITS_TQ0_SH_LITTLE;
    intern->tq1         = (ext->t_tq01[0] & TIR_BITS_TQ1_LITTLE)
			>>		    TIR_BITS_TQ1_SH_LITTLE;
    intern->tq2         = (ext->t_tq23[0] & TIR_BITS_TQ2_LITTLE)
			>>		    TIR_BITS_TQ2_SH_LITTLE;
    intern->tq3         = (ext->t_tq23[0] & TIR_BITS_TQ3_LITTLE)
			>>		    TIR_BITS_TQ3_SH_LITTLE;
  }

#ifdef TEST
  if (memcmp ((char *)ext, (char *)intern, sizeof (*intern)) != 0)
    abort();
#endif
}

/* Swap out a type information record.
   BIGEND says whether AUX symbols are big-endian or little-endian; this
   info comes from the file header record (fh-fBigendian).  */

void
ecoff_swap_tir_out (bigend, intern_copy, ext)
     int bigend;
     TIR *intern_copy;
     struct tir_ext *ext;
{
  TIR intern[1];

  *intern = *intern_copy;	/* Make it reasonable to do in-place.  */
  
  /* now the fun stuff... */
  if (bigend) {
    ext->t_bits1[0] = ((intern->fBitfield ? TIR_BITS1_FBITFIELD_BIG : 0)
		       | (intern->continued ? TIR_BITS1_CONTINUED_BIG : 0)
		       | ((intern->bt << TIR_BITS1_BT_SH_BIG)
			  & TIR_BITS1_BT_BIG));
    ext->t_tq45[0] = (((intern->tq4 << TIR_BITS_TQ4_SH_BIG)
		       & TIR_BITS_TQ4_BIG)
		      | ((intern->tq5 << TIR_BITS_TQ5_SH_BIG)
			 & TIR_BITS_TQ5_BIG));
    ext->t_tq01[0] = (((intern->tq0 << TIR_BITS_TQ0_SH_BIG)
		       & TIR_BITS_TQ0_BIG)
		      | ((intern->tq1 << TIR_BITS_TQ1_SH_BIG)
			 & TIR_BITS_TQ1_BIG));
    ext->t_tq23[0] = (((intern->tq2 << TIR_BITS_TQ2_SH_BIG)
		       & TIR_BITS_TQ2_BIG)
		      | ((intern->tq3 << TIR_BITS_TQ3_SH_BIG)
			 & TIR_BITS_TQ3_BIG));
  } else {
    ext->t_bits1[0] = ((intern->fBitfield ? TIR_BITS1_FBITFIELD_LITTLE : 0)
		       | (intern->continued ? TIR_BITS1_CONTINUED_LITTLE : 0)
		       | ((intern->bt << TIR_BITS1_BT_SH_LITTLE)
			  & TIR_BITS1_BT_LITTLE));
    ext->t_tq45[0] = (((intern->tq4 << TIR_BITS_TQ4_SH_LITTLE)
		       & TIR_BITS_TQ4_LITTLE)
		      | ((intern->tq5 << TIR_BITS_TQ5_SH_LITTLE)
			 & TIR_BITS_TQ5_LITTLE));
    ext->t_tq01[0] = (((intern->tq0 << TIR_BITS_TQ0_SH_LITTLE)
		       & TIR_BITS_TQ0_LITTLE)
		      | ((intern->tq1 << TIR_BITS_TQ1_SH_LITTLE)
			 & TIR_BITS_TQ1_LITTLE));
    ext->t_tq23[0] = (((intern->tq2 << TIR_BITS_TQ2_SH_LITTLE)
		       & TIR_BITS_TQ2_LITTLE)
		      | ((intern->tq3 << TIR_BITS_TQ3_SH_LITTLE)
			 & TIR_BITS_TQ3_LITTLE));
  }

#ifdef TEST
  if (memcmp ((char *)ext, (char *)intern, sizeof (*intern)) != 0)
    abort();
#endif
}

/* Swap in a relative symbol record.  BIGEND says whether it is in
   big-endian or little-endian format.*/

void
ecoff_swap_rndx_in (bigend, ext_copy, intern)
     int bigend;
     struct rndx_ext *ext_copy;
     RNDXR *intern;
{
  struct rndx_ext ext[1];

  *ext = *ext_copy;		/* Make it reasonable to do in-place.  */
  
  /* now the fun stuff... */
  if (bigend) {
    intern->rfd   = (ext->r_bits[0] << RNDX_BITS0_RFD_SH_LEFT_BIG)
		  | ((ext->r_bits[1] & RNDX_BITS1_RFD_BIG)
		    		    >> RNDX_BITS1_RFD_SH_BIG);
    intern->index = ((ext->r_bits[1] & RNDX_BITS1_INDEX_BIG)
		    		    << RNDX_BITS1_INDEX_SH_LEFT_BIG)
		  | (ext->r_bits[2] << RNDX_BITS2_INDEX_SH_LEFT_BIG)
		  | (ext->r_bits[3] << RNDX_BITS3_INDEX_SH_LEFT_BIG);
  } else {
    intern->rfd   = (ext->r_bits[0] << RNDX_BITS0_RFD_SH_LEFT_LITTLE)
		  | ((ext->r_bits[1] & RNDX_BITS1_RFD_LITTLE)
		    		    << RNDX_BITS1_RFD_SH_LEFT_LITTLE);
    intern->index = ((ext->r_bits[1] & RNDX_BITS1_INDEX_LITTLE)
		    		    >> RNDX_BITS1_INDEX_SH_LITTLE)
		  | (ext->r_bits[2] << RNDX_BITS2_INDEX_SH_LEFT_LITTLE)
		  | (ext->r_bits[3] << RNDX_BITS3_INDEX_SH_LEFT_LITTLE);
  }

#ifdef TEST
  if (memcmp ((char *)ext, (char *)intern, sizeof (*intern)) != 0)
    abort();
#endif
}

/* Swap out a relative symbol record.  BIGEND says whether it is in
   big-endian or little-endian format.*/

void
ecoff_swap_rndx_out (bigend, intern_copy, ext)
     int bigend;
     RNDXR *intern_copy;
     struct rndx_ext *ext;
{
  RNDXR intern[1];

  *intern = *intern_copy;	/* Make it reasonable to do in-place.  */
  
  /* now the fun stuff... */
  if (bigend) {
    ext->r_bits[0] = intern->rfd >> RNDX_BITS0_RFD_SH_LEFT_BIG;
    ext->r_bits[1] = (((intern->rfd << RNDX_BITS1_RFD_SH_BIG)
		       & RNDX_BITS1_RFD_BIG)
		      | ((intern->index >> RNDX_BITS1_INDEX_SH_LEFT_BIG)
			 & RNDX_BITS1_INDEX_BIG));
    ext->r_bits[2] = intern->index >> RNDX_BITS2_INDEX_SH_LEFT_BIG;
    ext->r_bits[3] = intern->index >> RNDX_BITS3_INDEX_SH_LEFT_BIG;
  } else {
    ext->r_bits[0] = intern->rfd >> RNDX_BITS0_RFD_SH_LEFT_LITTLE;
    ext->r_bits[1] = (((intern->rfd >> RNDX_BITS1_RFD_SH_LEFT_LITTLE)
		       & RNDX_BITS1_RFD_LITTLE)
		      | ((intern->index << RNDX_BITS1_INDEX_SH_LITTLE)
			 & RNDX_BITS1_INDEX_LITTLE));
    ext->r_bits[2] = intern->index >> RNDX_BITS2_INDEX_SH_LEFT_LITTLE;
    ext->r_bits[3] = intern->index >> RNDX_BITS3_INDEX_SH_LEFT_LITTLE;
  }

#ifdef TEST
  if (memcmp ((char *)ext, (char *)intern, sizeof (*intern)) != 0)
    abort();
#endif
}

/* Read in and swap the important symbolic information for an ECOFF
   object file.  This is called by gdb.  */

boolean
ecoff_slurp_symbolic_info (abfd)
     bfd *abfd;
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  bfd_size_type external_hdr_size;
  HDRR *internal_symhdr;
  bfd_size_type raw_base;
  bfd_size_type raw_size;
  PTR raw;
  bfd_size_type external_fdr_size;
  char *fraw_src;
  char *fraw_end;
  struct fdr *fdr_ptr;
  bfd_size_type raw_end;
  bfd_size_type cb_end;

  /* Check whether we've already gotten it, and whether there's any to
     get.  */
  if (ecoff_data (abfd)->raw_syments != (PTR) NULL)
    return true;
  if (ecoff_data (abfd)->sym_filepos == 0)
    {
      bfd_get_symcount (abfd) = 0;
      return true;
    }

  /* At this point bfd_get_symcount (abfd) holds the number of symbols
     as read from the file header, but on ECOFF this is always the
     size of the symbolic information header.  It would be cleaner to
     handle this when we first read the file in coffgen.c.  */
  external_hdr_size = backend->debug_swap.external_hdr_size;
  if (bfd_get_symcount (abfd) != external_hdr_size)
    {
      bfd_error = bad_value;
      return false;
    }

  /* Read the symbolic information header.  */
  raw = (PTR) alloca (external_hdr_size);
  if (bfd_seek (abfd, ecoff_data (abfd)->sym_filepos, SEEK_SET) == -1
      || (bfd_read (raw, external_hdr_size, 1, abfd)
	  != external_hdr_size))
    {
      bfd_error = system_call_error;
      return false;
    }
  internal_symhdr = &ecoff_data (abfd)->debug_info.symbolic_header;
  (*backend->debug_swap.swap_hdr_in) (abfd, raw, internal_symhdr);

  if (internal_symhdr->magic != backend->debug_swap.sym_magic)
    {
      bfd_error = bad_value;
      return false;
    }

  /* Now we can get the correct number of symbols.  */
  bfd_get_symcount (abfd) = (internal_symhdr->isymMax
			     + internal_symhdr->iextMax);

  /* Read all the symbolic information at once.  */
  raw_base = ecoff_data (abfd)->sym_filepos + external_hdr_size;

  /* Alpha ecoff makes the determination of raw_size difficult. It has
     an undocumented debug data section between the symhdr and the first
     documented section. And the ordering of the sections varies between
     statically and dynamically linked executables.
     If bfd supports SEEK_END someday, this code could be simplified.  */

  raw_end = 0;

#define UPDATE_RAW_END(start, count, size) \
  cb_end = internal_symhdr->start + internal_symhdr->count * (size); \
  if (cb_end > raw_end) \
    raw_end = cb_end

  UPDATE_RAW_END (cbLineOffset, cbLine, sizeof (unsigned char));
  UPDATE_RAW_END (cbDnOffset, idnMax, backend->debug_swap.external_dnr_size);
  UPDATE_RAW_END (cbPdOffset, ipdMax, backend->debug_swap.external_pdr_size);
  UPDATE_RAW_END (cbSymOffset, isymMax, backend->debug_swap.external_sym_size);
  UPDATE_RAW_END (cbOptOffset, ioptMax, backend->debug_swap.external_opt_size);
  UPDATE_RAW_END (cbAuxOffset, iauxMax, sizeof (union aux_ext));
  UPDATE_RAW_END (cbSsOffset, issMax, sizeof (char));
  UPDATE_RAW_END (cbSsExtOffset, issExtMax, sizeof (char));
  UPDATE_RAW_END (cbFdOffset, ifdMax, backend->debug_swap.external_fdr_size);
  UPDATE_RAW_END (cbRfdOffset, crfd, backend->debug_swap.external_rfd_size);
  UPDATE_RAW_END (cbExtOffset, iextMax, backend->debug_swap.external_ext_size);

#undef UPDATE_RAW_END

  raw_size = raw_end - raw_base;
  if (raw_size == 0)
    {
      ecoff_data (abfd)->sym_filepos = 0;
      return true;
    }
  raw = (PTR) bfd_alloc (abfd, raw_size);
  if (raw == NULL)
    {
      bfd_error = no_memory;
      return false;
    }
  if (bfd_read (raw, raw_size, 1, abfd) != raw_size)
    {
      bfd_error = system_call_error;
      bfd_release (abfd, raw);
      return false;
    }

  ecoff_data (abfd)->raw_syments = raw;

  /* Get pointers for the numeric offsets in the HDRR structure.  */
#define FIX(off1, off2, type) \
  if (internal_symhdr->off1 == 0) \
    ecoff_data (abfd)->debug_info.off2 = (type) NULL; \
  else \
    ecoff_data (abfd)->debug_info.off2 = (type) ((char *) raw \
						 + internal_symhdr->off1 \
						 - raw_base)
  FIX (cbLineOffset, line, unsigned char *);
  FIX (cbDnOffset, external_dnr, PTR);
  FIX (cbPdOffset, external_pdr, PTR);
  FIX (cbSymOffset, external_sym, PTR);
  FIX (cbOptOffset, external_opt, PTR);
  FIX (cbAuxOffset, external_aux, union aux_ext *);
  FIX (cbSsOffset, ss, char *);
  FIX (cbSsExtOffset, ssext, char *);
  FIX (cbFdOffset, external_fdr, PTR);
  FIX (cbRfdOffset, external_rfd, PTR);
  FIX (cbExtOffset, external_ext, PTR);
#undef FIX

  /* I don't want to always swap all the data, because it will just
     waste time and most programs will never look at it.  The only
     time the linker needs most of the debugging information swapped
     is when linking big-endian and little-endian MIPS object files
     together, which is not a common occurrence.

     We need to look at the fdr to deal with a lot of information in
     the symbols, so we swap them here.  */
  ecoff_data (abfd)->debug_info.fdr =
    (struct fdr *) bfd_alloc (abfd,
			      (internal_symhdr->ifdMax *
			       sizeof (struct fdr)));
  if (ecoff_data (abfd)->debug_info.fdr == NULL)
    {
      bfd_error = no_memory;
      return false;
    }
  external_fdr_size = backend->debug_swap.external_fdr_size;
  fdr_ptr = ecoff_data (abfd)->debug_info.fdr;
  fraw_src = (char *) ecoff_data (abfd)->debug_info.external_fdr;
  fraw_end = fraw_src + internal_symhdr->ifdMax * external_fdr_size;
  for (; fraw_src < fraw_end; fraw_src += external_fdr_size, fdr_ptr++)
    (*backend->debug_swap.swap_fdr_in) (abfd, (PTR) fraw_src, fdr_ptr);

  return true;
}

/* ECOFF symbol table routines.  The ECOFF symbol table is described
   in gcc/mips-tfile.c.  */

/* ECOFF uses two common sections.  One is the usual one, and the
   other is for small objects.  All the small objects are kept
   together, and then referenced via the gp pointer, which yields
   faster assembler code.  This is what we use for the small common
   section.  */
static asection ecoff_scom_section;
static asymbol ecoff_scom_symbol;
static asymbol *ecoff_scom_symbol_ptr;

/* Create an empty symbol.  */

asymbol *
ecoff_make_empty_symbol (abfd)
     bfd *abfd;
{
  ecoff_symbol_type *new;

  new = (ecoff_symbol_type *) bfd_alloc (abfd, sizeof (ecoff_symbol_type));
  if (new == (ecoff_symbol_type *) NULL)
    {
      bfd_error = no_memory;
      return (asymbol *) NULL;
    }
  memset (new, 0, sizeof *new);
  new->symbol.section = (asection *) NULL;
  new->fdr = (FDR *) NULL;
  new->local = false;
  new->native = NULL;
  new->symbol.the_bfd = abfd;
  return &new->symbol;
}

/* Set the BFD flags and section for an ECOFF symbol.  */

static void
ecoff_set_symbol_info (abfd, ecoff_sym, asym, ext, indirect_ptr_ptr)
     bfd *abfd;
     SYMR *ecoff_sym;
     asymbol *asym;
     int ext;
     asymbol **indirect_ptr_ptr;
{
  asym->the_bfd = abfd;
  asym->value = ecoff_sym->value;
  asym->section = &bfd_debug_section;
  asym->udata = NULL;

  /* An indirect symbol requires two consecutive stabs symbols.  */
  if (*indirect_ptr_ptr != (asymbol *) NULL)
    {
      BFD_ASSERT (ECOFF_IS_STAB (ecoff_sym));

      /* @@ Stuffing pointers into integers is a no-no.
	 We can usually get away with it if the integer is
	 large enough though.  */
      if (sizeof (asym) > sizeof (bfd_vma))
	abort ();
      (*indirect_ptr_ptr)->value = (bfd_vma) asym;

      asym->flags = BSF_DEBUGGING;
      asym->section = &bfd_und_section;
      *indirect_ptr_ptr = NULL;
      return;
    }

  if (ECOFF_IS_STAB (ecoff_sym)
      && (ECOFF_UNMARK_STAB (ecoff_sym->index) | N_EXT) == (N_INDR | N_EXT))
    {
      asym->flags = BSF_DEBUGGING | BSF_INDIRECT;
      asym->section = &bfd_ind_section;
      /* Pass this symbol on to the next call to this function.  */
      *indirect_ptr_ptr = asym;
      return;
    }

  /* Most symbol types are just for debugging.  */
  switch (ecoff_sym->st)
    {
    case stGlobal:
    case stStatic:
    case stLabel:
    case stProc:
    case stStaticProc:
      break;
    case stNil:
      if (ECOFF_IS_STAB (ecoff_sym))
	{
	  asym->flags = BSF_DEBUGGING;
	  return;
	}
      break;
    default:
      asym->flags = BSF_DEBUGGING;
      return;
    }

  if (ext)
    asym->flags = BSF_EXPORT | BSF_GLOBAL;
  else
    asym->flags = BSF_LOCAL;
  switch (ecoff_sym->sc)
    {
    case scNil:
      /* Used for compiler generated labels.  Leave them in the
	 debugging section, and mark them as local.  If BSF_DEBUGGING
	 is set, then nm does not display them for some reason.  If no
	 flags are set then the linker whines about them.  */
      asym->flags = BSF_LOCAL;
      break;
    case scText:
      asym->section = bfd_make_section_old_way (abfd, ".text");
      asym->value -= asym->section->vma;
      break;
    case scData:
      asym->section = bfd_make_section_old_way (abfd, ".data");
      asym->value -= asym->section->vma;
      break;
    case scBss:
      asym->section = bfd_make_section_old_way (abfd, ".bss");
      asym->value -= asym->section->vma;
      break;
    case scRegister:
      asym->flags = BSF_DEBUGGING;
      break;
    case scAbs:
      asym->section = &bfd_abs_section;
      break;
    case scUndefined:
      asym->section = &bfd_und_section;
      asym->flags = 0;
      asym->value = 0;
      break;
    case scCdbLocal:
    case scBits:
    case scCdbSystem:
    case scRegImage:
    case scInfo:
    case scUserStruct:
      asym->flags = BSF_DEBUGGING;
      break;
    case scSData:
      asym->section = bfd_make_section_old_way (abfd, ".sdata");
      asym->value -= asym->section->vma;
      break;
    case scSBss:
      asym->section = bfd_make_section_old_way (abfd, ".sbss");
      asym->value -= asym->section->vma;
      break;
    case scRData:
      asym->section = bfd_make_section_old_way (abfd, ".rdata");
      asym->value -= asym->section->vma;
      break;
    case scVar:
      asym->flags = BSF_DEBUGGING;
      break;
    case scCommon:
      if (asym->value > ecoff_data (abfd)->gp_size)
	{
	  asym->section = &bfd_com_section;
	  asym->flags = 0;
	  break;
	}
      /* Fall through.  */
    case scSCommon:
      if (ecoff_scom_section.name == NULL)
	{
	  /* Initialize the small common section.  */
	  ecoff_scom_section.name = SCOMMON;
	  ecoff_scom_section.flags = SEC_IS_COMMON;
	  ecoff_scom_section.output_section = &ecoff_scom_section;
	  ecoff_scom_section.symbol = &ecoff_scom_symbol;
	  ecoff_scom_section.symbol_ptr_ptr = &ecoff_scom_symbol_ptr;
	  ecoff_scom_symbol.name = SCOMMON;
	  ecoff_scom_symbol.flags = BSF_SECTION_SYM;
	  ecoff_scom_symbol.section = &ecoff_scom_section;
	  ecoff_scom_symbol_ptr = &ecoff_scom_symbol;
	}
      asym->section = &ecoff_scom_section;
      asym->flags = 0;
      break;
    case scVarRegister:
    case scVariant:
      asym->flags = BSF_DEBUGGING;
      break;
    case scSUndefined:
      asym->section = &bfd_und_section;
      asym->flags = 0;
      asym->value = 0;
      break;
    case scInit:
      asym->section = bfd_make_section_old_way (abfd, ".init");
      asym->value -= asym->section->vma;
      break;
    case scBasedVar:
    case scXData:
    case scPData:
      asym->flags = BSF_DEBUGGING;
      break;
    case scFini:
      asym->section = bfd_make_section_old_way (abfd, ".fini");
      asym->value -= asym->section->vma;
      break;
    default:
      break;
    }

  /* Look for special constructors symbols and make relocation entries
     in a special construction section.  These are produced by the
     -fgnu-linker argument to g++.  */
  if (ECOFF_IS_STAB (ecoff_sym))
    {
      switch (ECOFF_UNMARK_STAB (ecoff_sym->index))
	{
	default:
	  break;

	case N_SETA:
	case N_SETT:
	case N_SETD:
	case N_SETB:
	  {
	    const char *name;
	    asection *section;
	    arelent_chain *reloc_chain;
	    unsigned int bitsize;

	    /* Get a section with the same name as the symbol (usually
	       __CTOR_LIST__ or __DTOR_LIST__).  FIXME: gcc uses the
	       name ___CTOR_LIST (three underscores).  We need
	       __CTOR_LIST (two underscores), since ECOFF doesn't use
	       a leading underscore.  This should be handled by gcc,
	       but instead we do it here.  Actually, this should all
	       be done differently anyhow.  */
	    name = bfd_asymbol_name (asym);
	    if (name[0] == '_' && name[1] == '_' && name[2] == '_')
	      {
		++name;
		asym->name = name;
	      }
	    section = bfd_get_section_by_name (abfd, name);
	    if (section == (asection *) NULL)
	      {
		char *copy;

		copy = (char *) bfd_alloc (abfd, strlen (name) + 1);
		strcpy (copy, name);
		section = bfd_make_section (abfd, copy);
	      }

	    /* Build a reloc pointing to this constructor.  */
	    reloc_chain =
	      (arelent_chain *) bfd_alloc (abfd, sizeof (arelent_chain));
	    reloc_chain->relent.sym_ptr_ptr =
	      bfd_get_section (asym)->symbol_ptr_ptr;
	    reloc_chain->relent.address = section->_raw_size;
	    reloc_chain->relent.addend = asym->value;
	    reloc_chain->relent.howto =
	      ecoff_backend (abfd)->constructor_reloc;

	    /* Set up the constructor section to hold the reloc.  */
	    section->flags = SEC_CONSTRUCTOR;
	    ++section->reloc_count;

	    /* Constructor sections must be rounded to a boundary
	       based on the bitsize.  These are not real sections--
	       they are handled specially by the linker--so the ECOFF
	       16 byte alignment restriction does not apply.  */
	    bitsize = ecoff_backend (abfd)->constructor_bitsize;
	    section->alignment_power = 1;
	    while ((1 << section->alignment_power) < bitsize / 8)
	      ++section->alignment_power;

	    reloc_chain->next = section->constructor_chain;
	    section->constructor_chain = reloc_chain;
	    section->_raw_size += bitsize / 8;

	    /* Mark the symbol as a constructor.  */
	    asym->flags |= BSF_CONSTRUCTOR;
	  }
	  break;
	}
    }
}

/* Read an ECOFF symbol table.  */

boolean
ecoff_slurp_symbol_table (abfd)
     bfd *abfd;
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  const bfd_size_type external_ext_size
    = backend->debug_swap.external_ext_size;
  const bfd_size_type external_sym_size
    = backend->debug_swap.external_sym_size;
  void (* const swap_ext_in) PARAMS ((bfd *, PTR, EXTR *))
    = backend->debug_swap.swap_ext_in;
  void (* const swap_sym_in) PARAMS ((bfd *, PTR, SYMR *))
    = backend->debug_swap.swap_sym_in;
  bfd_size_type internal_size;
  ecoff_symbol_type *internal;
  ecoff_symbol_type *internal_ptr;
  asymbol *indirect_ptr;
  char *eraw_src;
  char *eraw_end;
  FDR *fdr_ptr;
  FDR *fdr_end;

  /* If we've already read in the symbol table, do nothing.  */
  if (ecoff_data (abfd)->canonical_symbols != NULL)
    return true;

  /* Get the symbolic information.  */
  if (ecoff_slurp_symbolic_info (abfd) == false)
    return false;
  if (bfd_get_symcount (abfd) == 0)
    return true;

  internal_size = bfd_get_symcount (abfd) * sizeof (ecoff_symbol_type);
  internal = (ecoff_symbol_type *) bfd_alloc (abfd, internal_size);
  if (internal == NULL)
    {
      bfd_error = no_memory;
      return false;
    }

  internal_ptr = internal;
  indirect_ptr = NULL;
  eraw_src = (char *) ecoff_data (abfd)->debug_info.external_ext;
  eraw_end = (eraw_src
	      + (ecoff_data (abfd)->debug_info.symbolic_header.iextMax
		 * external_ext_size));
  for (; eraw_src < eraw_end; eraw_src += external_ext_size, internal_ptr++)
    {
      EXTR internal_esym;

      (*swap_ext_in) (abfd, (PTR) eraw_src, &internal_esym);
      internal_ptr->symbol.name = (ecoff_data (abfd)->debug_info.ssext
				   + internal_esym.asym.iss);
      ecoff_set_symbol_info (abfd, &internal_esym.asym,
			     &internal_ptr->symbol, 1, &indirect_ptr);
      /* The alpha uses a negative ifd field for section symbols.  */
      if (internal_esym.ifd >= 0)
	internal_ptr->fdr = (ecoff_data (abfd)->debug_info.fdr
			     + internal_esym.ifd);
      else
	internal_ptr->fdr = NULL;
      internal_ptr->local = false;
      internal_ptr->native = (PTR) eraw_src;
    }
  BFD_ASSERT (indirect_ptr == (asymbol *) NULL);

  /* The local symbols must be accessed via the fdr's, because the
     string and aux indices are relative to the fdr information.  */
  fdr_ptr = ecoff_data (abfd)->debug_info.fdr;
  fdr_end = fdr_ptr + ecoff_data (abfd)->debug_info.symbolic_header.ifdMax;
  for (; fdr_ptr < fdr_end; fdr_ptr++)
    {
      char *lraw_src;
      char *lraw_end;

      lraw_src = ((char *) ecoff_data (abfd)->debug_info.external_sym
		  + fdr_ptr->isymBase * external_sym_size);
      lraw_end = lraw_src + fdr_ptr->csym * external_sym_size;
      for (;
	   lraw_src < lraw_end;
	   lraw_src += external_sym_size, internal_ptr++)
	{
	  SYMR internal_sym;

	  (*swap_sym_in) (abfd, (PTR) lraw_src, &internal_sym);
	  internal_ptr->symbol.name = (ecoff_data (abfd)->debug_info.ss
				       + fdr_ptr->issBase
				       + internal_sym.iss);
	  ecoff_set_symbol_info (abfd, &internal_sym,
				 &internal_ptr->symbol, 0, &indirect_ptr);
	  internal_ptr->fdr = fdr_ptr;
	  internal_ptr->local = true;
	  internal_ptr->native = (PTR) lraw_src;
	}
    }
  BFD_ASSERT (indirect_ptr == (asymbol *) NULL);

  ecoff_data (abfd)->canonical_symbols = internal;

  return true;
}

/* Return the amount of space needed for the canonical symbols.  */

unsigned int
ecoff_get_symtab_upper_bound (abfd)
     bfd *abfd;
{
  if (ecoff_slurp_symbolic_info (abfd) == false
      || bfd_get_symcount (abfd) == 0)
    return 0;

  return (bfd_get_symcount (abfd) + 1) * (sizeof (ecoff_symbol_type *));
}

/* Get the canonical symbols.  */

unsigned int
ecoff_get_symtab (abfd, alocation)
     bfd *abfd;
     asymbol **alocation;
{
  unsigned int counter = 0;
  ecoff_symbol_type *symbase;
  ecoff_symbol_type **location = (ecoff_symbol_type **) alocation;

  if (ecoff_slurp_symbol_table (abfd) == false
      || bfd_get_symcount (abfd) == 0)
    return 0;

  symbase = ecoff_data (abfd)->canonical_symbols;
  while (counter < bfd_get_symcount (abfd))
    {
      *(location++) = symbase++;
      counter++;
    }
  *location++ = (ecoff_symbol_type *) NULL;
  return bfd_get_symcount (abfd);
}

/* Turn ECOFF type information into a printable string.
   ecoff_emit_aggregate and ecoff_type_to_string are from
   gcc/mips-tdump.c, with swapping added and used_ptr removed.  */

/* Write aggregate information to a string.  */

static void
ecoff_emit_aggregate (abfd, string, rndx, isym, which)
     bfd *abfd;
     char *string;
     RNDXR *rndx;
     long isym;
     CONST char *which;
{
  int ifd = rndx->rfd;
  int indx = rndx->index;
  int sym_base, ss_base;
  CONST char *name;
  
  if (ifd == 0xfff)
    ifd = isym;

  sym_base = ecoff_data (abfd)->debug_info.fdr[ifd].isymBase;
  ss_base  = ecoff_data (abfd)->debug_info.fdr[ifd].issBase;
  
  if (indx == indexNil)
    name = "/* no name */";
  else
    {
      const struct ecoff_debug_swap * const debug_swap
	= &ecoff_backend (abfd)->debug_swap;
      SYMR sym;

      indx += sym_base;
      (*debug_swap->swap_sym_in)
	(abfd,
	 ((char *) ecoff_data (abfd)->debug_info.external_sym
	  + indx * debug_swap->external_sym_size),
	 &sym);
      name = ecoff_data (abfd)->debug_info.ss + ss_base + sym.iss;
    }

  sprintf (string,
	   "%s %s { ifd = %d, index = %ld }",
	   which, name, ifd,
	   ((long) indx
	    + ecoff_data (abfd)->debug_info.symbolic_header.iextMax));
}

/* Convert the type information to string format.  */

static char *
ecoff_type_to_string (abfd, aux_ptr, indx, bigendian)
     bfd *abfd;
     union aux_ext *aux_ptr;
     unsigned int indx;
     int bigendian;
{
  AUXU u;
  struct qual {
    unsigned int  type;
    int  low_bound;
    int  high_bound;
    int  stride;
  } qualifiers[7];

  unsigned int basic_type;
  int i;
  static char buffer1[1024];
  static char buffer2[1024];
  char *p1 = buffer1;
  char *p2 = buffer2;
  RNDXR rndx;

  for (i = 0; i < 7; i++)
    {
      qualifiers[i].low_bound = 0;
      qualifiers[i].high_bound = 0;
      qualifiers[i].stride = 0;
    }

  if (AUX_GET_ISYM (bigendian, &aux_ptr[indx]) == -1)
    return "-1 (no type)";
  ecoff_swap_tir_in (bigendian, &aux_ptr[indx++].a_ti, &u.ti);

  basic_type = u.ti.bt;
  qualifiers[0].type = u.ti.tq0;
  qualifiers[1].type = u.ti.tq1;
  qualifiers[2].type = u.ti.tq2;
  qualifiers[3].type = u.ti.tq3;
  qualifiers[4].type = u.ti.tq4;
  qualifiers[5].type = u.ti.tq5;
  qualifiers[6].type = tqNil;

  /*
   * Go get the basic type.
   */
  switch (basic_type)
    {
    case btNil:			/* undefined */
      strcpy (p1, "nil");
      break;

    case btAdr:			/* address - integer same size as pointer */
      strcpy (p1, "address");
      break;

    case btChar:		/* character */
      strcpy (p1, "char");
      break;

    case btUChar:		/* unsigned character */
      strcpy (p1, "unsigned char");
      break;

    case btShort:		/* short */
      strcpy (p1, "short");
      break;

    case btUShort:		/* unsigned short */
      strcpy (p1, "unsigned short");
      break;

    case btInt:			/* int */
      strcpy (p1, "int");
      break;

    case btUInt:		/* unsigned int */
      strcpy (p1, "unsigned int");
      break;

    case btLong:		/* long */
      strcpy (p1, "long");
      break;

    case btULong:		/* unsigned long */
      strcpy (p1, "unsigned long");
      break;

    case btFloat:		/* float (real) */
      strcpy (p1, "float");
      break;

    case btDouble:		/* Double (real) */
      strcpy (p1, "double");
      break;

      /* Structures add 1-2 aux words:
	 1st word is [ST_RFDESCAPE, offset] pointer to struct def;
	 2nd word is file index if 1st word rfd is ST_RFDESCAPE.  */

    case btStruct:		/* Structure (Record) */
      ecoff_swap_rndx_in (bigendian, &aux_ptr[indx].a_rndx, &rndx);
      ecoff_emit_aggregate (abfd, p1, &rndx,
			    (long) AUX_GET_ISYM (bigendian, &aux_ptr[indx+1]),
			    "struct");
      indx++;			/* skip aux words */
      break;

      /* Unions add 1-2 aux words:
	 1st word is [ST_RFDESCAPE, offset] pointer to union def;
	 2nd word is file index if 1st word rfd is ST_RFDESCAPE.  */

    case btUnion:		/* Union */
      ecoff_swap_rndx_in (bigendian, &aux_ptr[indx].a_rndx, &rndx);
      ecoff_emit_aggregate (abfd, p1, &rndx,
			    (long) AUX_GET_ISYM (bigendian, &aux_ptr[indx+1]),
			    "union");
      indx++;			/* skip aux words */
      break;

      /* Enumerations add 1-2 aux words:
	 1st word is [ST_RFDESCAPE, offset] pointer to enum def;
	 2nd word is file index if 1st word rfd is ST_RFDESCAPE.  */

    case btEnum:		/* Enumeration */
      ecoff_swap_rndx_in (bigendian, &aux_ptr[indx].a_rndx, &rndx);
      ecoff_emit_aggregate (abfd, p1, &rndx,
			    (long) AUX_GET_ISYM (bigendian, &aux_ptr[indx+1]),
			    "enum");
      indx++;			/* skip aux words */
      break;

    case btTypedef:		/* defined via a typedef, isymRef points */
      strcpy (p1, "typedef");
      break;

    case btRange:		/* subrange of int */
      strcpy (p1, "subrange");
      break;

    case btSet:			/* pascal sets */
      strcpy (p1, "set");
      break;

    case btComplex:		/* fortran complex */
      strcpy (p1, "complex");
      break;

    case btDComplex:		/* fortran double complex */
      strcpy (p1, "double complex");
      break;

    case btIndirect:		/* forward or unnamed typedef */
      strcpy (p1, "forward/unamed typedef");
      break;

    case btFixedDec:		/* Fixed Decimal */
      strcpy (p1, "fixed decimal");
      break;

    case btFloatDec:		/* Float Decimal */
      strcpy (p1, "float decimal");
      break;

    case btString:		/* Varying Length Character String */
      strcpy (p1, "string");
      break;

    case btBit:			/* Aligned Bit String */
      strcpy (p1, "bit");
      break;

    case btPicture:		/* Picture */
      strcpy (p1, "picture");
      break;

    case btVoid:		/* Void */
      strcpy (p1, "void");
      break;

    default:
      sprintf (p1, "Unknown basic type %d", (int) basic_type);
      break;
    }

  p1 += strlen (buffer1);

  /*
   * If this is a bitfield, get the bitsize.
   */
  if (u.ti.fBitfield)
    {
      int bitsize;

      bitsize = AUX_GET_WIDTH (bigendian, &aux_ptr[indx++]);
      sprintf (p1, " : %d", bitsize);
      p1 += strlen (buffer1);
    }


  /*
   * Deal with any qualifiers.
   */
  if (qualifiers[0].type != tqNil)
    {
      /*
       * Snarf up any array bounds in the correct order.  Arrays
       * store 5 successive words in the aux. table:
       *	word 0	RNDXR to type of the bounds (ie, int)
       *	word 1	Current file descriptor index
       *	word 2	low bound
       *	word 3	high bound (or -1 if [])
       *	word 4	stride size in bits
       */
      for (i = 0; i < 7; i++)
	{
	  if (qualifiers[i].type == tqArray)
	    {
	      qualifiers[i].low_bound =
		AUX_GET_DNLOW (bigendian, &aux_ptr[indx+2]);
	      qualifiers[i].high_bound =
		AUX_GET_DNHIGH (bigendian, &aux_ptr[indx+3]);
	      qualifiers[i].stride =
		AUX_GET_WIDTH (bigendian, &aux_ptr[indx+4]);
	      indx += 5;
	    }
	}

      /*
       * Now print out the qualifiers.
       */
      for (i = 0; i < 6; i++)
	{
	  switch (qualifiers[i].type)
	    {
	    case tqNil:
	    case tqMax:
	      break;

	    case tqPtr:
	      strcpy (p2, "ptr to ");
	      p2 += sizeof ("ptr to ")-1;
	      break;

	    case tqVol:
	      strcpy (p2, "volatile ");
	      p2 += sizeof ("volatile ")-1;
	      break;

	    case tqFar:
	      strcpy (p2, "far ");
	      p2 += sizeof ("far ")-1;
	      break;

	    case tqProc:
	      strcpy (p2, "func. ret. ");
	      p2 += sizeof ("func. ret. ");
	      break;

	    case tqArray:
	      {
		int first_array = i;
		int j;

		/* Print array bounds reversed (ie, in the order the C
		   programmer writes them).  C is such a fun language.... */

		while (i < 5 && qualifiers[i+1].type == tqArray)
		  i++;

		for (j = i; j >= first_array; j--)
		  {
		    strcpy (p2, "array [");
		    p2 += sizeof ("array [")-1;
		    if (qualifiers[j].low_bound != 0)
		      sprintf (p2,
			       "%ld:%ld {%ld bits}",
			       (long) qualifiers[j].low_bound,
			       (long) qualifiers[j].high_bound,
			       (long) qualifiers[j].stride);

		    else if (qualifiers[j].high_bound != -1)
		      sprintf (p2,
			       "%ld {%ld bits}",
			       (long) (qualifiers[j].high_bound + 1),
			       (long) (qualifiers[j].stride));

		    else
		      sprintf (p2, " {%ld bits}", (long) (qualifiers[j].stride));

		    p2 += strlen (p2);
		    strcpy (p2, "] of ");
		    p2 += sizeof ("] of ")-1;
		  }
	      }
	      break;
	    }
	}
    }

  strcpy (p2, buffer1);
  return buffer2;
}

/* Return information about ECOFF symbol SYMBOL in RET.  */

/*ARGSUSED*/
void
ecoff_get_symbol_info (abfd, symbol, ret)
     bfd *abfd;			/* Ignored.  */
     asymbol *symbol;
     symbol_info *ret;
{
  bfd_symbol_info (symbol, ret);
}

/* Print information about an ECOFF symbol.  */

void
ecoff_print_symbol (abfd, filep, symbol, how)
     bfd *abfd;
     PTR filep;
     asymbol *symbol;
     bfd_print_symbol_type how;
{
  const struct ecoff_debug_swap * const debug_swap
    = &ecoff_backend (abfd)->debug_swap;
  FILE *file = (FILE *)filep;

  switch (how)
    {
    case bfd_print_symbol_name:
      fprintf (file, "%s", symbol->name);
      break;
    case bfd_print_symbol_more:
      if (ecoffsymbol (symbol)->local)
	{
	  SYMR ecoff_sym;
	
	  (*debug_swap->swap_sym_in) (abfd, ecoffsymbol (symbol)->native,
				      &ecoff_sym);
	  fprintf (file, "ecoff local ");
	  fprintf_vma (file, (bfd_vma) ecoff_sym.value);
	  fprintf (file, " %x %x", (unsigned) ecoff_sym.st,
		   (unsigned) ecoff_sym.sc);
	}
      else
	{
	  EXTR ecoff_ext;

	  (*debug_swap->swap_ext_in) (abfd, ecoffsymbol (symbol)->native,
				      &ecoff_ext);
	  fprintf (file, "ecoff extern ");
	  fprintf_vma (file, (bfd_vma) ecoff_ext.asym.value);
	  fprintf (file, " %x %x", (unsigned) ecoff_ext.asym.st,
		   (unsigned) ecoff_ext.asym.sc);
	}
      break;
    case bfd_print_symbol_all:
      /* Print out the symbols in a reasonable way */
      {
	char type;
	int pos;
	EXTR ecoff_ext;
	char jmptbl;
	char cobol_main;
	char weakext;

	if (ecoffsymbol (symbol)->local)
	  {
	    (*debug_swap->swap_sym_in) (abfd, ecoffsymbol (symbol)->native,
					&ecoff_ext.asym);
	    type = 'l';
	    pos = ((((char *) ecoffsymbol (symbol)->native
		     - (char *) ecoff_data (abfd)->debug_info.external_sym)
		    / debug_swap->external_sym_size)
		   + ecoff_data (abfd)->debug_info.symbolic_header.iextMax);
	    jmptbl = ' ';
	    cobol_main = ' ';
	    weakext = ' ';
	  }
	else
	  {
	    (*debug_swap->swap_ext_in) (abfd, ecoffsymbol (symbol)->native,
					&ecoff_ext);
	    type = 'e';
	    pos = (((char *) ecoffsymbol (symbol)->native
		    - (char *) ecoff_data (abfd)->debug_info.external_ext)
		   / debug_swap->external_ext_size);
	    jmptbl = ecoff_ext.jmptbl ? 'j' : ' ';
	    cobol_main = ecoff_ext.cobol_main ? 'c' : ' ';
	    weakext = ecoff_ext.weakext ? 'w' : ' ';
	  }

	fprintf (file, "[%3d] %c ",
		 pos, type);
	fprintf_vma (file, (bfd_vma) ecoff_ext.asym.value);
	fprintf (file, " st %x sc %x indx %x %c%c%c %s",
		 (unsigned) ecoff_ext.asym.st,
		 (unsigned) ecoff_ext.asym.sc,
		 (unsigned) ecoff_ext.asym.index,
		 jmptbl, cobol_main, weakext,
		 symbol->name);

	if (ecoffsymbol (symbol)->fdr != NULL
	    && ecoff_ext.asym.index != indexNil)
	  {
	    unsigned int indx;
	    int bigendian;
	    bfd_size_type sym_base;
	    union aux_ext *aux_base;

	    indx = ecoff_ext.asym.index;

	    /* sym_base is used to map the fdr relative indices which
	       appear in the file to the position number which we are
	       using.  */
	    sym_base = ecoffsymbol (symbol)->fdr->isymBase;
	    if (ecoffsymbol (symbol)->local)
	      sym_base +=
		ecoff_data (abfd)->debug_info.symbolic_header.iextMax;

	    /* aux_base is the start of the aux entries for this file;
	       asym.index is an offset from this.  */
	    aux_base = (ecoff_data (abfd)->debug_info.external_aux
			+ ecoffsymbol (symbol)->fdr->iauxBase);

	    /* The aux entries are stored in host byte order; the
	       order is indicated by a bit in the fdr.  */
	    bigendian = ecoffsymbol (symbol)->fdr->fBigendian;

	    /* This switch is basically from gcc/mips-tdump.c  */
	    switch (ecoff_ext.asym.st)
	      {
	      case stNil:
	      case stLabel:
		break;

	      case stFile:
	      case stBlock:
		fprintf (file, "\n      End+1 symbol: %ld",
			 (long) (indx + sym_base));
		break;

	      case stEnd:
		if (ecoff_ext.asym.sc == scText
		    || ecoff_ext.asym.sc == scInfo)
		  fprintf (file, "\n      First symbol: %ld",
			   (long) (indx + sym_base));
		else
		  fprintf (file, "\n      First symbol: %ld", 
			   (long) (AUX_GET_ISYM (bigendian,
						 &aux_base[ecoff_ext.asym.index])
				   + sym_base));
		break;

	      case stProc:
	      case stStaticProc:
		if (ECOFF_IS_STAB (&ecoff_ext.asym))
		  ;
		else if (ecoffsymbol (symbol)->local)
		  fprintf (file, "\n      End+1 symbol: %-7ld   Type:  %s",
			   (long) (AUX_GET_ISYM (bigendian,
						 &aux_base[ecoff_ext.asym.index])
				   + sym_base),
			   ecoff_type_to_string (abfd, aux_base, indx + 1,
						 bigendian));
		else
		  fprintf (file, "\n      Local symbol: %ld",
			   ((long) indx
			    + (long) sym_base
			    + (ecoff_data (abfd)
			       ->debug_info.symbolic_header.iextMax)));
		break;

	      default:
		if (! ECOFF_IS_STAB (&ecoff_ext.asym))
		  fprintf (file, "\n      Type: %s",
			   ecoff_type_to_string (abfd, aux_base, indx,
						 bigendian));
		break;
	      }
	  }
      }
      break;
    }
}

/* Read in the relocs for a section.  */

static boolean
ecoff_slurp_reloc_table (abfd, section, symbols)
     bfd *abfd;
     asection *section;
     asymbol **symbols;
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  arelent *internal_relocs;
  bfd_size_type external_reloc_size;
  bfd_size_type external_relocs_size;
  char *external_relocs;
  arelent *rptr;
  unsigned int i;

  if (section->relocation != (arelent *) NULL
      || section->reloc_count == 0
      || (section->flags & SEC_CONSTRUCTOR) != 0)
    return true;

  if (ecoff_slurp_symbol_table (abfd) == false)
    return false;
  
  internal_relocs = (arelent *) bfd_alloc (abfd,
					   (sizeof (arelent)
					    * section->reloc_count));
  external_reloc_size = backend->external_reloc_size;
  external_relocs_size = external_reloc_size * section->reloc_count;
  external_relocs = (char *) bfd_alloc (abfd, external_relocs_size);
  if (internal_relocs == (arelent *) NULL
      || external_relocs == (char *) NULL)
    {
      bfd_error = no_memory;
      return false;
    }
  if (bfd_seek (abfd, section->rel_filepos, SEEK_SET) != 0)
    return false;
  if (bfd_read (external_relocs, 1, external_relocs_size, abfd)
      != external_relocs_size)
    {
      bfd_error = system_call_error;
      return false;
    }

  for (i = 0, rptr = internal_relocs; i < section->reloc_count; i++, rptr++)
    {
      struct internal_reloc intern;

      (*backend->swap_reloc_in) (abfd,
				 external_relocs + i * external_reloc_size,
				 &intern);

      if (intern.r_extern)
	{
	  /* r_symndx is an index into the external symbols.  */
	  BFD_ASSERT (intern.r_symndx >= 0
		      && (intern.r_symndx
			  < (ecoff_data (abfd)
			     ->debug_info.symbolic_header.iextMax)));
	  rptr->sym_ptr_ptr = symbols + intern.r_symndx;
	  rptr->addend = 0;
	}
      else if (intern.r_symndx == RELOC_SECTION_NONE
	       || intern.r_symndx == RELOC_SECTION_ABS)
	{
	  rptr->sym_ptr_ptr = bfd_abs_section.symbol_ptr_ptr;
	  rptr->addend = 0;
	}
      else
	{
	  CONST char *sec_name;
	  asection *sec;

	  /* r_symndx is a section key.  */
	  switch (intern.r_symndx)
	    {
	    case RELOC_SECTION_TEXT:  sec_name = ".text";  break;
	    case RELOC_SECTION_RDATA: sec_name = ".rdata"; break;
	    case RELOC_SECTION_DATA:  sec_name = ".data";  break;
	    case RELOC_SECTION_SDATA: sec_name = ".sdata"; break;
	    case RELOC_SECTION_SBSS:  sec_name = ".sbss";  break;
	    case RELOC_SECTION_BSS:   sec_name = ".bss";   break;
	    case RELOC_SECTION_INIT:  sec_name = ".init";  break;
	    case RELOC_SECTION_LIT8:  sec_name = ".lit8";  break;
	    case RELOC_SECTION_LIT4:  sec_name = ".lit4";  break;
	    case RELOC_SECTION_XDATA: sec_name = ".xdata"; break;
	    case RELOC_SECTION_PDATA: sec_name = ".pdata"; break;
	    case RELOC_SECTION_FINI:  sec_name = ".fini"; break;
	    case RELOC_SECTION_LITA:  sec_name = ".lita";  break;
	    default: abort ();
	    }

	  sec = bfd_get_section_by_name (abfd, sec_name);
	  if (sec == (asection *) NULL)
	    abort ();
	  rptr->sym_ptr_ptr = sec->symbol_ptr_ptr;

	  rptr->addend = - bfd_get_section_vma (abfd, sec);
	}

      rptr->address = intern.r_vaddr - bfd_get_section_vma (abfd, section);

      /* Let the backend select the howto field and do any other
	 required processing.  */
      (*backend->adjust_reloc_in) (abfd, &intern, rptr);
    }

  bfd_release (abfd, external_relocs);

  section->relocation = internal_relocs;

  return true;
}

/* Get a canonical list of relocs.  */

unsigned int
ecoff_canonicalize_reloc (abfd, section, relptr, symbols)
     bfd *abfd;
     asection *section;
     arelent **relptr;
     asymbol **symbols;
{
  unsigned int count;

  if (section->flags & SEC_CONSTRUCTOR) 
    {
      arelent_chain *chain;

      /* This section has relocs made up by us, not the file, so take
	 them out of their chain and place them into the data area
	 provided.  */
      for (count = 0, chain = section->constructor_chain;
	   count < section->reloc_count;
	   count++, chain = chain->next)
	*relptr++ = &chain->relent;
    }
  else
    { 
      arelent *tblptr;

      if (ecoff_slurp_reloc_table (abfd, section, symbols) == false)
	return 0;

      tblptr = section->relocation;
      if (tblptr == (arelent *) NULL)
	return 0;

      for (count = 0; count < section->reloc_count; count++)
	*relptr++ = tblptr++;
    }

  *relptr = (arelent *) NULL;

  return section->reloc_count;
}

/* Provided a BFD, a section and an offset into the section, calculate
   and return the name of the source file and the line nearest to the
   wanted location.  */

/*ARGSUSED*/
boolean
ecoff_find_nearest_line (abfd,
			 section,
			 ignore_symbols,
			 offset,
			 filename_ptr,
			 functionname_ptr,
			 retline_ptr)
     bfd *abfd;
     asection *section;
     asymbol **ignore_symbols;
     bfd_vma offset;
     CONST char **filename_ptr;
     CONST char **functionname_ptr;
     unsigned int *retline_ptr;
{
  const struct ecoff_debug_swap * const debug_swap
    = &ecoff_backend (abfd)->debug_swap;
  FDR *fdr_ptr;
  FDR *fdr_start;
  FDR *fdr_end;
  FDR *fdr_hold;
  bfd_size_type external_pdr_size;
  char *pdr_ptr;
  char *pdr_end;
  PDR pdr;
  unsigned char *line_ptr;
  unsigned char *line_end;
  int lineno;

  /* If we're not in the .text section, we don't have any line
     numbers.  */
  if (strcmp (section->name, _TEXT) != 0
      || offset < ecoff_data (abfd)->text_start
      || offset >= ecoff_data (abfd)->text_end)
    return false;

  /* Make sure we have the FDR's.  */
  if (ecoff_slurp_symbolic_info (abfd) == false
      || bfd_get_symcount (abfd) == 0)
    return false;

  /* Each file descriptor (FDR) has a memory address.  Here we track
     down which FDR we want.  The FDR's are stored in increasing
     memory order.  If speed is ever important, this can become a
     binary search.  We must ignore FDR's with no PDR entries; they
     will have the adr of the FDR before or after them.  */
  fdr_start = ecoff_data (abfd)->debug_info.fdr;
  fdr_end = fdr_start + ecoff_data (abfd)->debug_info.symbolic_header.ifdMax;
  fdr_hold = (FDR *) NULL;
  for (fdr_ptr = fdr_start; fdr_ptr < fdr_end; fdr_ptr++)
    {
      if (fdr_ptr->cpd == 0)
	continue;
      if (offset < fdr_ptr->adr)
	break;
      fdr_hold = fdr_ptr;
    }
  if (fdr_hold == (FDR *) NULL)
    return false;
  fdr_ptr = fdr_hold;

  /* Each FDR has a list of procedure descriptors (PDR).  PDR's also
     have an address, which is relative to the FDR address, and are
     also stored in increasing memory order.  */
  offset -= fdr_ptr->adr;
  external_pdr_size = debug_swap->external_pdr_size;
  pdr_ptr = ((char *) ecoff_data (abfd)->debug_info.external_pdr
	     + fdr_ptr->ipdFirst * external_pdr_size);
  pdr_end = pdr_ptr + fdr_ptr->cpd * external_pdr_size;
  (*debug_swap->swap_pdr_in) (abfd, (PTR) pdr_ptr, &pdr);

  /* The address of the first PDR is an offset which applies to the
     addresses of all the PDR's.  */
  offset += pdr.adr;

  for (pdr_ptr += external_pdr_size;
       pdr_ptr < pdr_end;
       pdr_ptr += external_pdr_size)
    {
      (*debug_swap->swap_pdr_in) (abfd, (PTR) pdr_ptr, &pdr);
      if (offset < pdr.adr)
	break;
    }

  /* Now we can look for the actual line number.  The line numbers are
     stored in a very funky format, which I won't try to describe.
     Note that right here pdr_ptr and pdr hold the PDR *after* the one
     we want; we need this to compute line_end.  */
  line_end = ecoff_data (abfd)->debug_info.line;
  if (pdr_ptr == pdr_end)
    line_end += fdr_ptr->cbLineOffset + fdr_ptr->cbLine;
  else
    line_end += fdr_ptr->cbLineOffset + pdr.cbLineOffset;

  /* Now change pdr and pdr_ptr to the one we want.  */
  pdr_ptr -= external_pdr_size;
  (*debug_swap->swap_pdr_in) (abfd, (PTR) pdr_ptr, &pdr);

  offset -= pdr.adr;
  lineno = pdr.lnLow;
  line_ptr = (ecoff_data (abfd)->debug_info.line
	      + fdr_ptr->cbLineOffset
	      + pdr.cbLineOffset);
  while (line_ptr < line_end)
    {
      int delta;
      int count;

      delta = *line_ptr >> 4;
      if (delta >= 0x8)
	delta -= 0x10;
      count = (*line_ptr & 0xf) + 1;
      ++line_ptr;
      if (delta == -8)
	{
	  delta = (((line_ptr[0]) & 0xff) << 8) + ((line_ptr[1]) & 0xff);
	  if (delta >= 0x8000)
	    delta -= 0x10000;
	  line_ptr += 2;
	}
      lineno += delta;
      if (offset < count * 4)
	break;
      offset -= count * 4;
    }

  /* If fdr_ptr->rss is -1, then this file does not have full symbols,
     at least according to gdb/mipsread.c.  */
  if (fdr_ptr->rss == -1)
    {
      *filename_ptr = NULL;
      if (pdr.isym == -1)
	*functionname_ptr = NULL;
      else
	{
	  EXTR proc_ext;

	  (*debug_swap->swap_ext_in)
	    (abfd,
	     ((char *) ecoff_data (abfd)->debug_info.external_ext
	      + pdr.isym * debug_swap->external_ext_size),
	     &proc_ext);
	  *functionname_ptr = (ecoff_data (abfd)->debug_info.ssext
			       + proc_ext.asym.iss);
	}
    }
  else
    {
      SYMR proc_sym;

      *filename_ptr = (ecoff_data (abfd)->debug_info.ss
		       + fdr_ptr->issBase
		       + fdr_ptr->rss);
      (*debug_swap->swap_sym_in)
	(abfd,
	 ((char *) ecoff_data (abfd)->debug_info.external_sym
	  + (fdr_ptr->isymBase + pdr.isym) * debug_swap->external_sym_size),
	 &proc_sym);
      *functionname_ptr = (ecoff_data (abfd)->debug_info.ss
			   + fdr_ptr->issBase
			   + proc_sym.iss);
    }
  if (lineno == ilineNil)
    lineno = 0;
  *retline_ptr = lineno;
  return true;
}

/* We can't use the generic linking routines for ECOFF, because we
   have to handle all the debugging information.  The generic link
   routine just works out the section contents and attaches a list of
   symbols.  We find each input BFD by looping over all the link_order
   information.  We accumulate the debugging information for each
   input BFD.  */

/* Get ECOFF EXTR information for an external symbol.  This function
   is passed to bfd_ecoff_debug_externals.  */

static boolean
ecoff_get_extr (sym, esym)
     asymbol *sym;
     EXTR *esym;
{
  ecoff_symbol_type *ecoff_sym_ptr;
  bfd *input_bfd;

  /* Don't include debugging or local symbols.  */
  if ((sym->flags & BSF_DEBUGGING) != 0
      || (sym->flags & BSF_LOCAL) != 0)
    return false;

  if (bfd_asymbol_flavour (sym) != bfd_target_ecoff_flavour
      || ecoffsymbol (sym)->native == NULL)
    {
      esym->jmptbl = 0;
      esym->cobol_main = 0;
      esym->weakext = 0;
      esym->reserved = 0;
      esym->ifd = ifdNil;
      /* FIXME: we can do better than this for st and sc.  */
      esym->asym.st = stGlobal;
      esym->asym.sc = scAbs;
      esym->asym.reserved = 0;
      esym->asym.index = indexNil;
      return true;
    }

  ecoff_sym_ptr = ecoffsymbol (sym);

  if (ecoff_sym_ptr->local)
    abort ();

  input_bfd = bfd_asymbol_bfd (sym);
  (*(ecoff_backend (input_bfd)->debug_swap.swap_ext_in))
    (input_bfd, ecoff_sym_ptr->native, esym);

  /* If the symbol was defined by the linker, then esym will be
     undefined but sym will not be.  Get a better class for such a
     symbol.  */
  if ((esym->asym.sc == scUndefined
       || esym->asym.sc == scSUndefined)
      && bfd_get_section (sym) != &bfd_und_section)
    esym->asym.sc = scAbs;

  /* Adjust the FDR index for the symbol by that used for the input
     BFD.  */
  esym->ifd += ecoff_data (input_bfd)->debug_info.ifdbase;

  return true;
}

/* Set the external symbol index.  This routine is passed to
   bfd_ecoff_debug_externals.  */

static void
ecoff_set_index (sym, indx)
     asymbol *sym;
     bfd_size_type indx;
{
  ecoff_set_sym_index (sym, indx);
}

/* This is the actual link routine.  It builds the debugging
   information, and then lets the generic linking routine complete the
   link.  */

boolean
ecoff_bfd_final_link (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  struct ecoff_debug_info * const debug = &ecoff_data (abfd)->debug_info;
  HDRR *symhdr;
  register bfd *input_bfd;
  asection *o;

  /* We accumulate the debugging information counts in the symbolic
     header.  */
  symhdr = &debug->symbolic_header;
  symhdr->magic = backend->debug_swap.sym_magic;
  /* FIXME: What should the version stamp be?  */
  symhdr->vstamp = 0;
  symhdr->ilineMax = 0;
  symhdr->cbLine = 0;
  symhdr->idnMax = 0;
  symhdr->ipdMax = 0;
  symhdr->isymMax = 0;
  symhdr->ioptMax = 0;
  symhdr->iauxMax = 0;
  symhdr->issMax = 0;
  symhdr->ifdMax = 0;
  symhdr->crfd = 0;

  /* We accumulate the debugging information itself in the debug_info
     structure.  */
  debug->line = debug->line_end = NULL;
  debug->external_dnr = debug->external_dnr_end = NULL;
  debug->external_pdr = debug->external_pdr_end = NULL;
  debug->external_sym = debug->external_sym_end = NULL;
  debug->external_opt = debug->external_opt_end = NULL;
  debug->external_aux = debug->external_aux_end = NULL;
  debug->ss = debug->ss_end = NULL;
  debug->external_fdr = debug->external_fdr_end = NULL;
  debug->external_rfd = debug->external_rfd_end = NULL;

  /* We accumulate the debugging symbols from each input BFD.  */
  for (input_bfd = info->input_bfds;
       input_bfd != (bfd *) NULL;
       input_bfd = input_bfd->link_next)
    {
      boolean ret;

      if (bfd_get_flavour (input_bfd) == bfd_target_ecoff_flavour)
	ret = (bfd_ecoff_debug_accumulate
	       (abfd, debug, &backend->debug_swap,
		input_bfd, &ecoff_data (input_bfd)->debug_info,
		&ecoff_backend (input_bfd)->debug_swap, info->relocateable));
      else
	ret = bfd_ecoff_debug_link_other (abfd,
					  debug,
					  &backend->debug_swap,
					  input_bfd);

      if (! ret)
	return false;

      /* Combine the register masks.  */
      ecoff_data (abfd)->gprmask |= ecoff_data (input_bfd)->gprmask;
      ecoff_data (abfd)->fprmask |= ecoff_data (input_bfd)->fprmask;
      ecoff_data (abfd)->cprmask[0] |= ecoff_data (input_bfd)->cprmask[0];
      ecoff_data (abfd)->cprmask[1] |= ecoff_data (input_bfd)->cprmask[1];
      ecoff_data (abfd)->cprmask[2] |= ecoff_data (input_bfd)->cprmask[2];
      ecoff_data (abfd)->cprmask[3] |= ecoff_data (input_bfd)->cprmask[3];
    }

  /* Don't let the generic routine link the .reginfo sections.  */
  for (o = abfd->sections; o != (asection *) NULL; o = o->next)
    {
      if (strcmp (o->name, REGINFO) == 0)
	{
	  o->link_order_head = (struct bfd_link_order *) NULL;
	  break;
	}
    }

  /* Let the generic link routine handle writing out the section
     contents.  */
  return _bfd_generic_final_link (abfd, info);
}

/* Set the architecture.  The supported architecture is stored in the
   backend pointer.  We always set the architecture anyhow, since many
   callers ignore the return value.  */

boolean
ecoff_set_arch_mach (abfd, arch, machine)
     bfd *abfd;
     enum bfd_architecture arch;
     unsigned long machine;
{
  bfd_default_set_arch_mach (abfd, arch, machine);
  return arch == ecoff_backend (abfd)->arch;
}

/* Get the size of the section headers.  We do not output the .reginfo
   section.  */

/*ARGSUSED*/
int
ecoff_sizeof_headers (abfd, reloc)
     bfd *abfd;
     boolean reloc;
{
  asection *current;
  int c;

  c = 0;
  for (current = abfd->sections;
       current != (asection *)NULL; 
       current = current->next) 
    if (strcmp (current->name, REGINFO) != 0)
      ++c;

  return (bfd_coff_filhsz (abfd)
	  + bfd_coff_aoutsz (abfd)
	  + c * bfd_coff_scnhsz (abfd));
}

/* Get the contents of a section.  This is where we handle reading the
   .reginfo section, which implicitly holds the contents of an
   ecoff_reginfo structure.  */

boolean
ecoff_get_section_contents (abfd, section, location, offset, count)
     bfd *abfd;
     asection *section;
     PTR location;
     file_ptr offset;
     bfd_size_type count;
{
  ecoff_data_type *tdata = ecoff_data (abfd);
  struct ecoff_reginfo s;
  int i;

  if (strcmp (section->name, REGINFO) != 0)
    return bfd_generic_get_section_contents (abfd, section, location,
					     offset, count);

  s.gp_value = tdata->gp;
  s.gprmask = tdata->gprmask;
  for (i = 0; i < 4; i++)
    s.cprmask[i] = tdata->cprmask[i];
  s.fprmask = tdata->fprmask;

  /* bfd_get_section_contents has already checked that the offset and
     size is reasonable.  We don't have to worry about swapping or any
     such thing; the .reginfo section is defined such that the
     contents are an ecoff_reginfo structure as seen on the host.  */
  memcpy (location, ((char *) &s) + offset, (size_t) count);
  return true;
}

/* Calculate the file position for each section, and set
   reloc_filepos.  */

static void
ecoff_compute_section_file_positions (abfd)
     bfd *abfd;
{
  asection *current;
  file_ptr sofar;
  file_ptr old_sofar;
  boolean first_data;

  sofar = ecoff_sizeof_headers (abfd, false);

  first_data = true;
  for (current = abfd->sections;
       current != (asection *) NULL;
       current = current->next)
    {
      /* Only deal with sections which have contents */
      if ((current->flags & (SEC_HAS_CONTENTS | SEC_LOAD)) == 0
	  || strcmp (current->name, REGINFO) == 0)
	continue;

      /* On Ultrix, the data sections in an executable file must be
	 aligned to a page boundary within the file.  This does not
	 affect the section size, though.  FIXME: Does this work for
	 other platforms?  It requires some modification for the
	 Alpha, because .rdata on the Alpha goes with the text, not
	 the data.  */
      if ((abfd->flags & EXEC_P) != 0
	  && (abfd->flags & D_PAGED) != 0
	  && first_data != false
	  && (current->flags & SEC_CODE) == 0
	  && (! ecoff_backend (abfd)->rdata_in_text
	      || strcmp (current->name, _RDATA) != 0)
	  && strcmp (current->name, _PDATA) != 0)
	{
	  const bfd_vma round = ecoff_backend (abfd)->round;

	  sofar = (sofar + round - 1) &~ (round - 1);
	  first_data = false;
	}

      /* Align the sections in the file to the same boundary on
	 which they are aligned in virtual memory.  */
      old_sofar = sofar;
      sofar = BFD_ALIGN (sofar, 1 << current->alignment_power);

      current->filepos = sofar;

      sofar += current->_raw_size;

      /* make sure that this section is of the right size too */
      old_sofar = sofar;
      sofar = BFD_ALIGN (sofar, 1 << current->alignment_power);
      current->_raw_size += sofar - old_sofar;
    }

  ecoff_data (abfd)->reloc_filepos = sofar;
}

/* Set the contents of a section.  This is where we handle setting the
   contents of the .reginfo section, which implicitly holds a
   ecoff_reginfo structure.  */

boolean
ecoff_set_section_contents (abfd, section, location, offset, count)
     bfd *abfd;
     asection *section;
     PTR location;
     file_ptr offset;
     bfd_size_type count;
{
  if (abfd->output_has_begun == false)
    ecoff_compute_section_file_positions (abfd);

  if (strcmp (section->name, REGINFO) == 0)
    {
      ecoff_data_type *tdata = ecoff_data (abfd);
      struct ecoff_reginfo s;
      int i;

      /* If the caller is only changing part of the structure, we must
	 retrieve the current information before the memcpy.  */
      if (offset != 0 || count != sizeof (struct ecoff_reginfo))
	{
	  s.gp_value = tdata->gp;
	  s.gprmask = tdata->gprmask;
	  for (i = 0; i < 4; i++)
	    s.cprmask[i] = tdata->cprmask[i];
	  s.fprmask = tdata->fprmask;
	}

      /* bfd_set_section_contents has already checked that the offset
	 and size is reasonable.  We don't have to worry about
	 swapping or any such thing; the .reginfo section is defined
	 such that the contents are an ecoff_reginfo structure as seen
	 on the host.  */
      memcpy (((char *) &s) + offset, location, (size_t) count);

      tdata->gp = s.gp_value;
      tdata->gprmask = s.gprmask;
      for (i = 0; i < 4; i++)
	tdata->cprmask[i] = s.cprmask[i];
      tdata->fprmask = s.fprmask;

      return true;

    }

  bfd_seek (abfd, (file_ptr) (section->filepos + offset), SEEK_SET);

  if (count != 0)
    return (bfd_write (location, 1, count, abfd) == count) ? true : false;

  return true;
}

/* Write out an ECOFF file.  */

boolean
ecoff_write_object_contents (abfd)
     bfd *abfd;
{
  const struct ecoff_backend_data * const backend = ecoff_backend (abfd);
  const bfd_vma round = backend->round;
  const bfd_size_type filhsz = bfd_coff_filhsz (abfd);
  const bfd_size_type aoutsz = bfd_coff_aoutsz (abfd);
  const bfd_size_type scnhsz = bfd_coff_scnhsz (abfd);
  const bfd_size_type external_hdr_size
    = backend->debug_swap.external_hdr_size;
  const bfd_size_type external_reloc_size = backend->external_reloc_size;
  void (* const adjust_reloc_out) PARAMS ((bfd *,
					   const arelent *,
					   struct internal_reloc *))
    = backend->adjust_reloc_out;
  void (* const swap_reloc_out) PARAMS ((bfd *,
					 const struct internal_reloc *,
					 PTR))
    = backend->swap_reloc_out;
  struct ecoff_debug_info * const debug = &ecoff_data (abfd)->debug_info;
  HDRR * const symhdr = &debug->symbolic_header;
  asection *current;
  unsigned int count;
  file_ptr reloc_base;
  file_ptr sym_base;
  unsigned long reloc_size;
  unsigned long text_size;
  unsigned long text_start;
  unsigned long data_size;
  unsigned long data_start;
  unsigned long bss_size;
  PTR buff;
  struct internal_filehdr internal_f;
  struct internal_aouthdr internal_a;
  int i;

  bfd_error = system_call_error;

  if(abfd->output_has_begun == false)
    ecoff_compute_section_file_positions(abfd);

  reloc_base = ecoff_data (abfd)->reloc_filepos;

  count = 1;
  reloc_size = 0;
  for (current = abfd->sections;
       current != (asection *)NULL; 
       current = current->next) 
    {
      if (strcmp (current->name, REGINFO) == 0)
	continue;
      current->target_index = count;
      ++count;
      if (current->reloc_count != 0)
	{
	  bfd_size_type relsize;

	  current->rel_filepos = reloc_base;
	  relsize = current->reloc_count * external_reloc_size;
	  reloc_size += relsize;
	  reloc_base += relsize;
	}
      else
	current->rel_filepos = 0;
    }

  sym_base = reloc_base + reloc_size;

  /* At least on Ultrix, the symbol table of an executable file must
     be aligned to a page boundary.  FIXME: Is this true on other
     platforms?  */
  if ((abfd->flags & EXEC_P) != 0
      && (abfd->flags & D_PAGED) != 0)
    sym_base = (sym_base + round - 1) &~ (round - 1);

  ecoff_data (abfd)->sym_filepos = sym_base;

  if ((abfd->flags & D_PAGED) != 0)
    text_size = ecoff_sizeof_headers (abfd, false);
  else
    text_size = 0;
  text_start = 0;
  data_size = 0;
  data_start = 0;
  bss_size = 0;

  /* Write section headers to the file.  */

  buff = (PTR) alloca (scnhsz);
  internal_f.f_nscns = 0;
  if (bfd_seek (abfd, (file_ptr) (filhsz + aoutsz), SEEK_SET) != 0)
    return false;
  for (current = abfd->sections;
       current != (asection *) NULL;
       current = current->next)
    {
      struct internal_scnhdr section;
      bfd_vma vma;

      if (strcmp (current->name, REGINFO) == 0)
	{
	  BFD_ASSERT (current->reloc_count == 0);
	  continue;
	}

      ++internal_f.f_nscns;

      strncpy (section.s_name, current->name, sizeof section.s_name);

      /* FIXME: is this correct for shared libraries?  I think it is
	 but I have no platform to check.  Ian Lance Taylor.  */
      vma = bfd_get_section_vma (abfd, current);
      if (strcmp (current->name, _LIB) == 0)
	section.s_vaddr = 0;
      else
	section.s_vaddr = vma;

      section.s_paddr = vma;
      section.s_size = bfd_get_section_size_before_reloc (current);

      /* If this section is unloadable then the scnptr will be 0.  */
      if ((current->flags & (SEC_LOAD | SEC_HAS_CONTENTS)) == 0)
	section.s_scnptr = 0;
      else
	section.s_scnptr = current->filepos;
      section.s_relptr = current->rel_filepos;

      /* FIXME: the lnnoptr of the .sbss or .sdata section of an
	 object file produced by the assembler is supposed to point to
	 information about how much room is required by objects of
	 various different sizes.  I think this only matters if we
	 want the linker to compute the best size to use, or
	 something.  I don't know what happens if the information is
	 not present.  */
      section.s_lnnoptr = 0;

      section.s_nreloc = current->reloc_count;
      section.s_nlnno = 0;
      section.s_flags = ecoff_sec_to_styp_flags (current->name,
						 current->flags);

      bfd_coff_swap_scnhdr_out (abfd, (PTR) &section, buff);
      if (bfd_write (buff, 1, scnhsz, abfd) != scnhsz)
	return false;

      if ((section.s_flags & STYP_TEXT) != 0
	  || ((section.s_flags & STYP_RDATA) != 0
	      && backend->rdata_in_text)
	  || strcmp (current->name, _PDATA) == 0)
	{
	  text_size += bfd_get_section_size_before_reloc (current);
	  if (text_start == 0 || text_start > vma)
	    text_start = vma;
	}
      else if ((section.s_flags & STYP_RDATA) != 0
	       || (section.s_flags & STYP_DATA) != 0
	       || (section.s_flags & STYP_LITA) != 0
	       || (section.s_flags & STYP_LIT8) != 0
	       || (section.s_flags & STYP_LIT4) != 0
	       || (section.s_flags & STYP_SDATA) != 0)
	{
	  data_size += bfd_get_section_size_before_reloc (current);
	  if (data_start == 0 || data_start > vma)
	    data_start = vma;
	}
      else if ((section.s_flags & STYP_BSS) != 0
	       || (section.s_flags & STYP_SBSS) != 0)
	bss_size += bfd_get_section_size_before_reloc (current);
    }	

  /* Set up the file header.  */

  internal_f.f_magic = ecoff_get_magic (abfd);

  /* We will NOT put a fucking timestamp in the header here. Every
     time you put it back, I will come in and take it out again.  I'm
     sorry.  This field does not belong here.  We fill it with a 0 so
     it compares the same but is not a reasonable time. --
     gnu@cygnus.com.  */
  internal_f.f_timdat = 0;

  if (bfd_get_symcount (abfd) != 0)
    {
      /* The ECOFF f_nsyms field is not actually the number of
	 symbols, it's the size of symbolic information header.  */
      internal_f.f_nsyms = external_hdr_size;
      internal_f.f_symptr = sym_base;
    }
  else
    {
      internal_f.f_nsyms = 0;
      internal_f.f_symptr = 0;
    }

  internal_f.f_opthdr = aoutsz;

  internal_f.f_flags = F_LNNO;
  if (reloc_size == 0)
    internal_f.f_flags |= F_RELFLG;
  if (bfd_get_symcount (abfd) == 0)
    internal_f.f_flags |= F_LSYMS;
  if (abfd->flags & EXEC_P)
    internal_f.f_flags |= F_EXEC;

  if (! abfd->xvec->byteorder_big_p)
    internal_f.f_flags |= F_AR32WR;
  else
    internal_f.f_flags |= F_AR32W;

  /* Set up the ``optional'' header.  */
  if ((abfd->flags & D_PAGED) != 0)
    internal_a.magic = ECOFF_AOUT_ZMAGIC;
  else
    internal_a.magic = ECOFF_AOUT_OMAGIC;

  /* FIXME: This is what Ultrix puts in, and it makes the Ultrix
     linker happy.  But, is it right?  */
  internal_a.vstamp = 0x20a;

  /* At least on Ultrix, these have to be rounded to page boundaries.
     FIXME: Is this true on other platforms?  */
  if ((abfd->flags & D_PAGED) != 0)
    {
      internal_a.tsize = (text_size + round - 1) &~ (round - 1);
      internal_a.text_start = text_start &~ (round - 1);
      internal_a.dsize = (data_size + round - 1) &~ (round - 1);
      internal_a.data_start = data_start &~ (round - 1);
    }
  else
    {
      internal_a.tsize = text_size;
      internal_a.text_start = text_start;
      internal_a.dsize = data_size;
      internal_a.data_start = data_start;
    }

  /* On Ultrix, the initial portions of the .sbss and .bss segments
     are at the end of the data section.  The bsize field in the
     optional header records how many bss bytes are required beyond
     those in the data section.  The value is not rounded to a page
     boundary.  */
  if (bss_size < internal_a.dsize - data_size)
    bss_size = 0;
  else
    bss_size -= internal_a.dsize - data_size;
  internal_a.bsize = bss_size;
  internal_a.bss_start = internal_a.data_start + internal_a.dsize;

  internal_a.entry = bfd_get_start_address (abfd);

  internal_a.gp_value = ecoff_data (abfd)->gp;

  internal_a.gprmask = ecoff_data (abfd)->gprmask;
  internal_a.fprmask = ecoff_data (abfd)->fprmask;
  for (i = 0; i < 4; i++)
    internal_a.cprmask[i] = ecoff_data (abfd)->cprmask[i];

  /* Write out the file header and the optional header.  */

  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
    return false;

  buff = (PTR) alloca (filhsz);
  bfd_coff_swap_filehdr_out (abfd, (PTR) &internal_f, buff);
  if (bfd_write (buff, 1, filhsz, abfd) != filhsz)
    return false;

  buff = (PTR) alloca (aoutsz);
  bfd_coff_swap_aouthdr_out (abfd, (PTR) &internal_a, buff);
  if (bfd_write (buff, 1, aoutsz, abfd) != aoutsz)
    return false;

  /* Build the external symbol information.  This must be done before
     writing out the relocs so that we know the symbol indices.  */
  symhdr->iextMax = 0;
  symhdr->issExtMax = 0;
  debug->external_ext = debug->external_ext_end = NULL;
  debug->ssext = debug->ssext_end = NULL;
  if (bfd_ecoff_debug_externals (abfd, debug, &backend->debug_swap,
				 (((abfd->flags & EXEC_P) == 0)
				  ? true : false),
				 ecoff_get_extr, ecoff_set_index)
      == false)
    return false;

  /* Write out the relocs.  */
  for (current = abfd->sections;
       current != (asection *) NULL;
       current = current->next)
    {
      arelent **reloc_ptr_ptr;
      arelent **reloc_end;
      char *out_ptr;

      if (current->reloc_count == 0)
	continue;

      buff = bfd_alloc (abfd, current->reloc_count * external_reloc_size);
      if (buff == NULL)
	{
	  bfd_error = no_memory;
	  return false;
	}

      reloc_ptr_ptr = current->orelocation;
      reloc_end = reloc_ptr_ptr + current->reloc_count;
      out_ptr = (char *) buff;
      for (;
	   reloc_ptr_ptr < reloc_end;
	   reloc_ptr_ptr++, out_ptr += external_reloc_size)
	{
	  arelent *reloc;
	  asymbol *sym;
	  struct internal_reloc in;
	  
	  memset (&in, 0, sizeof in);

	  reloc = *reloc_ptr_ptr;
	  sym = *reloc->sym_ptr_ptr;

	  in.r_vaddr = reloc->address + bfd_get_section_vma (abfd, current);
	  in.r_type = reloc->howto->type;

	  if ((sym->flags & BSF_SECTION_SYM) == 0)
	    {
	      in.r_symndx = ecoff_get_sym_index (*reloc->sym_ptr_ptr);
	      in.r_extern = 1;
	    }
	  else
	    {
	      CONST char *name;

	      name = bfd_get_section_name (abfd, bfd_get_section (sym));
	      if (strcmp (name, ".text") == 0)
		in.r_symndx = RELOC_SECTION_TEXT;
	      else if (strcmp (name, ".rdata") == 0)
		in.r_symndx = RELOC_SECTION_RDATA;
	      else if (strcmp (name, ".data") == 0)
		in.r_symndx = RELOC_SECTION_DATA;
	      else if (strcmp (name, ".sdata") == 0)
		in.r_symndx = RELOC_SECTION_SDATA;
	      else if (strcmp (name, ".sbss") == 0)
		in.r_symndx = RELOC_SECTION_SBSS;
	      else if (strcmp (name, ".bss") == 0)
		in.r_symndx = RELOC_SECTION_BSS;
	      else if (strcmp (name, ".init") == 0)
		in.r_symndx = RELOC_SECTION_INIT;
	      else if (strcmp (name, ".lit8") == 0)
		in.r_symndx = RELOC_SECTION_LIT8;
	      else if (strcmp (name, ".lit4") == 0)
		in.r_symndx = RELOC_SECTION_LIT4;
	      else if (strcmp (name, ".xdata") == 0)
		in.r_symndx = RELOC_SECTION_XDATA;
	      else if (strcmp (name, ".pdata") == 0)
		in.r_symndx = RELOC_SECTION_PDATA;
	      else if (strcmp (name, ".fini") == 0)
		in.r_symndx = RELOC_SECTION_FINI;
	      else if (strcmp (name, ".lita") == 0)
		in.r_symndx = RELOC_SECTION_LITA;
	      else if (strcmp (name, "*ABS*") == 0)
		in.r_symndx = RELOC_SECTION_ABS;
	      else
		abort ();
	      in.r_extern = 0;
	    }

	  (*adjust_reloc_out) (abfd, reloc, &in);

	  (*swap_reloc_out) (abfd, &in, (PTR) out_ptr);
	}

      if (bfd_seek (abfd, current->rel_filepos, SEEK_SET) != 0)
	return false;
      if (bfd_write (buff, external_reloc_size, current->reloc_count, abfd)
	  != external_reloc_size * current->reloc_count)
	return false;
      bfd_release (abfd, buff);
    }

  /* Write out the symbolic debugging information.  */
  if (bfd_get_symcount (abfd) > 0)
    {
      /* Write out the debugging information.  */
      if (bfd_ecoff_write_debug (abfd, debug, &backend->debug_swap,
				 ecoff_data (abfd)->sym_filepos)
	  == false)
	return false;
    }
  else if ((abfd->flags & EXEC_P) != 0
	   && (abfd->flags & D_PAGED) != 0)
    {
      char c;

      /* A demand paged executable must occupy an even number of
	 pages.  */
      if (bfd_seek (abfd, (file_ptr) ecoff_data (abfd)->sym_filepos - 1,
		    SEEK_SET) != 0)
	return false;
      if (bfd_read (&c, 1, 1, abfd) == 0)
	c = 0;
      if (bfd_seek (abfd, (file_ptr) ecoff_data (abfd)->sym_filepos - 1,
		    SEEK_SET) != 0)
	return false;
      if (bfd_write (&c, 1, 1, abfd) != 1)
	return false;      
    }

  return true;
}

/* Archive handling.  ECOFF uses what appears to be a unique type of
   archive header (armap).  The byte ordering of the armap and the
   contents are encoded in the name of the armap itself.  At least for
   now, we only support archives with the same byte ordering in the
   armap and the contents.

   The first four bytes in the armap are the number of symbol
   definitions.  This is always a power of two.

   This is followed by the symbol definitions.  Each symbol definition
   occupies 8 bytes.  The first four bytes are the offset from the
   start of the armap strings to the null-terminated string naming
   this symbol.  The second four bytes are the file offset to the
   archive member which defines this symbol.  If the second four bytes
   are 0, then this is not actually a symbol definition, and it should
   be ignored.

   The symbols are hashed into the armap with a closed hashing scheme.
   See the functions below for the details of the algorithm.

   After the symbol definitions comes four bytes holding the size of
   the string table, followed by the string table itself.  */

/* The name of an archive headers looks like this:
   __________E[BL]E[BL]_ (with a trailing space).
   The trailing space is changed to an X if the archive is changed to
   indicate that the armap is out of date.

   The Alpha seems to use ________64E[BL]E[BL]_.  */

#define ARMAP_BIG_ENDIAN 'B'
#define ARMAP_LITTLE_ENDIAN 'L'
#define ARMAP_MARKER 'E'
#define ARMAP_START_LENGTH 10
#define ARMAP_HEADER_MARKER_INDEX 10
#define ARMAP_HEADER_ENDIAN_INDEX 11
#define ARMAP_OBJECT_MARKER_INDEX 12
#define ARMAP_OBJECT_ENDIAN_INDEX 13
#define ARMAP_END_INDEX 14
#define ARMAP_END "_ "

/* This is a magic number used in the hashing algorithm.  */
#define ARMAP_HASH_MAGIC 0x9dd68ab5

/* This returns the hash value to use for a string.  It also sets
   *REHASH to the rehash adjustment if the first slot is taken.  SIZE
   is the number of entries in the hash table, and HLOG is the log
   base 2 of SIZE.  */

static unsigned int
ecoff_armap_hash (s, rehash, size, hlog)
     CONST char *s;
     unsigned int *rehash;
     unsigned int size;
     unsigned int hlog;
{
  unsigned int hash;

  hash = *s++;
  while (*s != '\0')
    hash = ((hash >> 27) | (hash << 5)) + *s++;
  hash *= ARMAP_HASH_MAGIC;
  *rehash = (hash & (size - 1)) | 1;
  return hash >> (32 - hlog);
}

/* Read in the armap.  */

boolean
ecoff_slurp_armap (abfd)
     bfd *abfd;
{
  char nextname[17];
  unsigned int i;
  struct areltdata *mapdata;
  bfd_size_type parsed_size;
  char *raw_armap;
  struct artdata *ardata;
  unsigned int count;
  char *raw_ptr;
  struct symdef *symdef_ptr;
  char *stringbase;
  
  /* Get the name of the first element.  */
  i = bfd_read ((PTR) nextname, 1, 16, abfd);
  if (i == 0)
      return true;
  if (i != 16)
      return false;

  bfd_seek (abfd, (file_ptr) -16, SEEK_CUR);

  /* Irix 4.0.5F apparently can use either an ECOFF armap or a
     standard COFF armap.  We could move the ECOFF armap stuff into
     bfd_slurp_armap, but that seems inappropriate since no other
     target uses this format.  Instead, we check directly for a COFF
     armap.  */
  if (strncmp (nextname, "/               ", 16) == 0)
    return bfd_slurp_armap (abfd);

  /* See if the first element is an armap.  */
  if (strncmp (nextname, ecoff_backend (abfd)->armap_start,
	       ARMAP_START_LENGTH) != 0
      || nextname[ARMAP_HEADER_MARKER_INDEX] != ARMAP_MARKER
      || (nextname[ARMAP_HEADER_ENDIAN_INDEX] != ARMAP_BIG_ENDIAN
	  && nextname[ARMAP_HEADER_ENDIAN_INDEX] != ARMAP_LITTLE_ENDIAN)
      || nextname[ARMAP_OBJECT_MARKER_INDEX] != ARMAP_MARKER
      || (nextname[ARMAP_OBJECT_ENDIAN_INDEX] != ARMAP_BIG_ENDIAN
	  && nextname[ARMAP_OBJECT_ENDIAN_INDEX] != ARMAP_LITTLE_ENDIAN)
      || strncmp (nextname + ARMAP_END_INDEX,
		  ARMAP_END, sizeof ARMAP_END - 1) != 0)
    {
      bfd_has_map (abfd) = false;
      return true;
    }

  /* Make sure we have the right byte ordering.  */
  if (((nextname[ARMAP_HEADER_ENDIAN_INDEX] == ARMAP_BIG_ENDIAN)
       ^ (abfd->xvec->header_byteorder_big_p != false))
      || ((nextname[ARMAP_OBJECT_ENDIAN_INDEX] == ARMAP_BIG_ENDIAN)
	  ^ (abfd->xvec->byteorder_big_p != false)))
    {
      bfd_error = wrong_format;
      return false;
    }

  /* Read in the armap.  */
  ardata = bfd_ardata (abfd);
  mapdata = _bfd_snarf_ar_hdr (abfd);
  if (mapdata == (struct areltdata *) NULL)
    return false;
  parsed_size = mapdata->parsed_size;
  bfd_release (abfd, (PTR) mapdata);
    
  raw_armap = (char *) bfd_alloc (abfd, parsed_size);
  if (raw_armap == (char *) NULL)
    {
      bfd_error = no_memory;
      return false;
    }
    
  if (bfd_read ((PTR) raw_armap, 1, parsed_size, abfd) != parsed_size)
    {
      bfd_error = malformed_archive;
      bfd_release (abfd, (PTR) raw_armap);
      return false;
    }
    
  ardata->tdata = (PTR) raw_armap;

  count = bfd_h_get_32 (abfd, (PTR) raw_armap);

  ardata->symdef_count = 0;
  ardata->cache = (struct ar_cache *) NULL;

  /* This code used to overlay the symdefs over the raw archive data,
     but that doesn't work on a 64 bit host.  */

  stringbase = raw_armap + count * 8 + 8;

#ifdef CHECK_ARMAP_HASH
  {
    unsigned int hlog;

    /* Double check that I have the hashing algorithm right by making
       sure that every symbol can be looked up successfully.  */
    hlog = 0;
    for (i = 1; i < count; i <<= 1)
      hlog++;
    BFD_ASSERT (i == count);

    raw_ptr = raw_armap + 4;
    for (i = 0; i < count; i++, raw_ptr += 8)
      {
	unsigned int name_offset, file_offset;
	unsigned int hash, rehash, srch;
      
	name_offset = bfd_h_get_32 (abfd, (PTR) raw_ptr);
	file_offset = bfd_h_get_32 (abfd, (PTR) (raw_ptr + 4));
	if (file_offset == 0)
	  continue;
	hash = ecoff_armap_hash (stringbase + name_offset, &rehash, count,
				 hlog);
	if (hash == i)
	  continue;

	/* See if we can rehash to this location.  */
	for (srch = (hash + rehash) & (count - 1);
	     srch != hash && srch != i;
	     srch = (srch + rehash) & (count - 1))
	  BFD_ASSERT (bfd_h_get_32 (abfd, (PTR) (raw_armap + 8 + srch * 8))
		      != 0);
	BFD_ASSERT (srch == i);
      }
  }

#endif /* CHECK_ARMAP_HASH */

  raw_ptr = raw_armap + 4;
  for (i = 0; i < count; i++, raw_ptr += 8)
    if (bfd_h_get_32 (abfd, (PTR) (raw_ptr + 4)) != 0)
      ++ardata->symdef_count;

  symdef_ptr = ((struct symdef *)
		bfd_alloc (abfd,
			   ardata->symdef_count * sizeof (struct symdef)));
  ardata->symdefs = (carsym *) symdef_ptr;

  raw_ptr = raw_armap + 4;
  for (i = 0; i < count; i++, raw_ptr += 8)
    {
      unsigned int name_offset, file_offset;

      file_offset = bfd_h_get_32 (abfd, (PTR) (raw_ptr + 4));
      if (file_offset == 0)
	continue;
      name_offset = bfd_h_get_32 (abfd, (PTR) raw_ptr);
      symdef_ptr->s.name = stringbase + name_offset;
      symdef_ptr->file_offset = file_offset;
      ++symdef_ptr;
    }

  ardata->first_file_filepos = bfd_tell (abfd);
  /* Pad to an even boundary.  */
  ardata->first_file_filepos += ardata->first_file_filepos % 2;

  bfd_has_map (abfd) = true;

  return true;
}

/* Write out an armap.  */

boolean
ecoff_write_armap (abfd, elength, map, orl_count, stridx)
     bfd *abfd;
     unsigned int elength;
     struct orl *map;
     unsigned int orl_count;
     int stridx;
{
  unsigned int hashsize, hashlog;
  unsigned int symdefsize;
  int padit;
  unsigned int stringsize;
  unsigned int mapsize;
  file_ptr firstreal;
  struct ar_hdr hdr;
  struct stat statbuf;
  unsigned int i;
  bfd_byte temp[4];
  bfd_byte *hashtable;
  bfd *current;
  bfd *last_elt;

  /* Ultrix appears to use as a hash table size the least power of two
     greater than twice the number of entries.  */
  for (hashlog = 0; (1 << hashlog) <= 2 * orl_count; hashlog++)
    ;
  hashsize = 1 << hashlog;

  symdefsize = hashsize * 8;
  padit = stridx % 2;
  stringsize = stridx + padit;

  /* Include 8 bytes to store symdefsize and stringsize in output. */
  mapsize = symdefsize + stringsize + 8;

  firstreal = SARMAG + sizeof (struct ar_hdr) + mapsize + elength;

  memset ((PTR) &hdr, 0, sizeof hdr);

  /* Work out the ECOFF armap name.  */
  strcpy (hdr.ar_name, ecoff_backend (abfd)->armap_start);
  hdr.ar_name[ARMAP_HEADER_MARKER_INDEX] = ARMAP_MARKER;
  hdr.ar_name[ARMAP_HEADER_ENDIAN_INDEX] =
    (abfd->xvec->header_byteorder_big_p
     ? ARMAP_BIG_ENDIAN
     : ARMAP_LITTLE_ENDIAN);
  hdr.ar_name[ARMAP_OBJECT_MARKER_INDEX] = ARMAP_MARKER;
  hdr.ar_name[ARMAP_OBJECT_ENDIAN_INDEX] =
    abfd->xvec->byteorder_big_p ? ARMAP_BIG_ENDIAN : ARMAP_LITTLE_ENDIAN;
  memcpy (hdr.ar_name + ARMAP_END_INDEX, ARMAP_END, sizeof ARMAP_END - 1);

  /* Write the timestamp of the archive header to be just a little bit
     later than the timestamp of the file, otherwise the linker will
     complain that the index is out of date.  Actually, the Ultrix
     linker just checks the archive name; the GNU linker may check the
     date.  */
  stat (abfd->filename, &statbuf);
  sprintf (hdr.ar_date, "%ld", (long) (statbuf.st_mtime + 60));

  /* The DECstation uses zeroes for the uid, gid and mode of the
     armap.  */
  hdr.ar_uid[0] = '0';
  hdr.ar_gid[0] = '0';
  hdr.ar_mode[0] = '0';

  sprintf (hdr.ar_size, "%-10d", (int) mapsize);

  hdr.ar_fmag[0] = '`';
  hdr.ar_fmag[1] = '\n';

  /* Turn all null bytes in the header into spaces.  */
  for (i = 0; i < sizeof (struct ar_hdr); i++)
   if (((char *)(&hdr))[i] == '\0')
     (((char *)(&hdr))[i]) = ' ';

  if (bfd_write ((PTR) &hdr, 1, sizeof (struct ar_hdr), abfd)
      != sizeof (struct ar_hdr))
    return false;

  bfd_h_put_32 (abfd, (bfd_vma) hashsize, temp);
  if (bfd_write ((PTR) temp, 1, 4, abfd) != 4)
    return false;
  
  hashtable = (bfd_byte *) bfd_zalloc (abfd, symdefsize);

  current = abfd->archive_head;
  last_elt = current;
  for (i = 0; i < orl_count; i++)
    {
      unsigned int hash, rehash;

      /* Advance firstreal to the file position of this archive
	 element.  */
      if (((bfd *) map[i].pos) != last_elt)
	{
	  do
	    {
	      firstreal += arelt_size (current) + sizeof (struct ar_hdr);
	      firstreal += firstreal % 2;
	      current = current->next;
	    }
	  while (current != (bfd *) map[i].pos);
	}

      last_elt = current;

      hash = ecoff_armap_hash (*map[i].name, &rehash, hashsize, hashlog);
      if (bfd_h_get_32 (abfd, (PTR) (hashtable + (hash * 8) + 4)) != 0)
	{
	  unsigned int srch;

	  /* The desired slot is already taken.  */
	  for (srch = (hash + rehash) & (hashsize - 1);
	       srch != hash;
	       srch = (srch + rehash) & (hashsize - 1))
	    if (bfd_h_get_32 (abfd, (PTR) (hashtable + (srch * 8) + 4)) == 0)
	      break;

	  BFD_ASSERT (srch != hash);

	  hash = srch;
	}
	
      bfd_h_put_32 (abfd, (bfd_vma) map[i].namidx,
		    (PTR) (hashtable + hash * 8));
      bfd_h_put_32 (abfd, (bfd_vma) firstreal,
		    (PTR) (hashtable + hash * 8 + 4));
    }

  if (bfd_write ((PTR) hashtable, 1, symdefsize, abfd) != symdefsize)
    return false;

  bfd_release (abfd, hashtable);

  /* Now write the strings.  */
  bfd_h_put_32 (abfd, (bfd_vma) stringsize, temp);
  if (bfd_write ((PTR) temp, 1, 4, abfd) != 4)
    return false;
  for (i = 0; i < orl_count; i++)
    {
      bfd_size_type len;

      len = strlen (*map[i].name) + 1;
      if (bfd_write ((PTR) (*map[i].name), 1, len, abfd) != len)
	return false;
    }

  /* The spec sez this should be a newline.  But in order to be
     bug-compatible for DECstation ar we use a null.  */
  if (padit)
    {
      if (bfd_write ("", 1, 1, abfd) != 1)
	return false;
    }

  return true;
}

/* See whether this BFD is an archive.  If it is, read in the armap
   and the extended name table.  */

bfd_target *
ecoff_archive_p (abfd)
     bfd *abfd;
{
  char armag[SARMAG + 1];

  if (bfd_read ((PTR) armag, 1, SARMAG, abfd) != SARMAG
      || strncmp (armag, ARMAG, SARMAG) != 0)
    {
      bfd_error = wrong_format;
      return (bfd_target *) NULL;
    }

  /* We are setting bfd_ardata(abfd) here, but since bfd_ardata
     involves a cast, we can't do it as the left operand of
     assignment.  */
  abfd->tdata.aout_ar_data =
    (struct artdata *) bfd_zalloc (abfd, sizeof (struct artdata));

  if (bfd_ardata (abfd) == (struct artdata *) NULL)
    {
      bfd_error = no_memory;
      return (bfd_target *) NULL;
    }

  bfd_ardata (abfd)->first_file_filepos = SARMAG;
  bfd_ardata (abfd)->cache = NULL;
  bfd_ardata (abfd)->archive_head = NULL;
  bfd_ardata (abfd)->symdefs = NULL;
  bfd_ardata (abfd)->extended_names = NULL;
  bfd_ardata (abfd)->tdata = NULL;
  
  if (ecoff_slurp_armap (abfd) == false
      || ecoff_slurp_extended_name_table (abfd) == false)
    {
      bfd_release (abfd, bfd_ardata (abfd));
      abfd->tdata.aout_ar_data = (struct artdata *) NULL;
      return (bfd_target *) NULL;
    }
  
  return abfd->xvec;
}
