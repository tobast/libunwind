/* libunwind - a platform-independent unwind library
   Copyright (c) 2003 Hewlett-Packard Development Company, L.P.
        Contributed by David Mosberger-Tang <davidm@hpl.hp.com>

   Modified for x86_64 by Max Asbock <masbock@us.ibm.com>

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include "init.h"
#include "../eh_elf/eh_elf.h"
#include "unwind_i.h"

int init_id = 0;

PROTECTED int
unw_init_remote (unw_cursor_t *cursor, unw_addr_space_t as, void *as_arg)
{
#ifdef UNW_LOCAL_ONLY
  return -UNW_EINVAL;
#else /* !UNW_LOCAL_ONLY */
  struct cursor *c = (struct cursor *) cursor;
  int ret;
  struct unw_eh_elf_init_acc* eh_elf_acc = &as->acc.eh_elf_init;

  if (!tdep_init_done)
    tdep_init ();

  init_id++;
  Debug (1, "(init_id=%d, cursor=%p)\n", init_id, c);

  switch(eh_elf_acc->init_mode) {
      case UNW_EH_ELF_INIT_PID:
          ret = eh_elf_init_pid(eh_elf_acc->init_data.get_pid(as_arg));
          if(ret < 0)
              return ret;
          break;
      case UNW_EH_ELF_INIT_MMAP: {
          unw_mmap_entry_t* entries;
          size_t entries_count;
          eh_elf_acc->init_data.get_mmap(&entries, &entries_count, as_arg);
          ret = eh_elf_init_mmap(entries, entries_count);
          free(entries);
          if(ret < 0)
              return ret;
          break;
      }
  }

  c->dwarf.as = as;
  if (as == unw_local_addr_space)
    {
      c->dwarf.as_arg = c;
      c->uc = as_arg;
    }
  else
    {
      c->dwarf.as_arg = as_arg;
      c->uc = NULL;
    }
  return common_init (c, 0);
#endif /* !UNW_LOCAL_ONLY */
}
