/* Copyright (C) 2014 Embecosm Limited and University of Bristol

   Contributor James Pallister <james.pallister@bristol.ac.uk>

   This file is part of the Bristol/Embecosm Embedded Benchmark Suite.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>. */

#include <support.h>
#include <stdint.h>

/* Helpers to access machine counters. */
static inline uint64_t
read_mcycle (void)
{
  uint64_t value;
  asm volatile ("csrr %0, mcycle" : "=r" (value));
  return value;
}

static inline uint64_t
read_minstret (void)
{
  uint64_t value;
  asm volatile ("csrr %0, minstret" : "=r" (value));
  return value;
}

static uint64_t
compute_text_size (void)
{
  uintptr_t start = 0;
  uintptr_t end = 0;

  extern char __text_start[] __attribute__ ((weak));
  extern char __text_end[] __attribute__ ((weak));
  extern char __TEXT_START__[] __attribute__ ((weak));
  extern char __TEXT_END__[] __attribute__ ((weak));
  extern char _start[] __attribute__ ((weak));
  extern char _etext[] __attribute__ ((weak));
  extern char __executable_start[] __attribute__ ((weak));

  if (&__text_start != 0 && &__text_end != 0)
    {
      start = (uintptr_t) __text_start;
      end = (uintptr_t) __text_end;
    }
  else if (&__TEXT_START__ != 0 && &__TEXT_END__ != 0)
    {
      start = (uintptr_t) __TEXT_START__;
      end = (uintptr_t) __TEXT_END__;
    }
  else if (&__executable_start != 0 && &_etext != 0)
    {
      start = (uintptr_t) __executable_start;
      end = (uintptr_t) _etext;
    }
  else if (&_start != 0 && &_etext != 0)
    {
      start = (uintptr_t) _start;
      end = (uintptr_t) _etext;
    }

  return (end > start) ? (end - start) : 0;
}

void
initialise_board (void)
{
  beebs_metrics.mcycle_start = 0;
  beebs_metrics.mcycle_end = 0;
  beebs_metrics.minstret_start = 0;
  beebs_metrics.minstret_end = 0;
  beebs_metrics.cycles = 0;
  beebs_metrics.retired_instructions = 0;
  beebs_metrics.ipc_numerator = 0;
  beebs_metrics.ipc_denominator = 1;
  beebs_metrics.text_size = compute_text_size ();
}

void
start_trigger (void)
{
  beebs_metrics.mcycle_start = read_mcycle ();
  beebs_metrics.minstret_start = read_minstret ();
}

void
stop_trigger (void)
{
  const uint64_t mcycle = read_mcycle ();
  const uint64_t minstret = read_minstret ();

  beebs_metrics.mcycle_end = mcycle;
  beebs_metrics.minstret_end = minstret;

  beebs_metrics.cycles = mcycle - beebs_metrics.mcycle_start;
  beebs_metrics.retired_instructions = minstret - beebs_metrics.minstret_start;

  if (beebs_metrics.cycles != 0)
    {
      beebs_metrics.ipc_numerator = beebs_metrics.retired_instructions;
      beebs_metrics.ipc_denominator = beebs_metrics.cycles;
    }
  else
    {
      beebs_metrics.ipc_numerator = 0;
      beebs_metrics.ipc_denominator = 1;
    }
}

