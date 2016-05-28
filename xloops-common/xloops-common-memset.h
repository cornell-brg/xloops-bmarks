//========================================================================
// xloops-common-memset
//========================================================================
// memset function taken from llvm library implementation with
// modifications to use XLOOPS instructions

#ifndef XLOOPS_COMMON_MEMSET_H
#define XLOOPS_COMMON_MEMSET_H

#include <cstring>

__attribute__ ((noinline))
void *memset_xloops( void *ptr, int value, size_t num ) {
  #ifdef _MIPS_ARCH_MAVEN
  // this is taken from the memset llvm uses
  __asm__ volatile(
            "   andi    $v1,$a0,0x3    \n "
            "   move    $v0,$a0        \n "
            "   beqz    $v1,3f         \n "
            "   beqz    $a2,10f        \n "
            "   sll     $a3,$a1,0x18   \n "
            "   addiu   $a2,$a2,-1     \n "
            "   sra     $a3,$a3,0x18   \n "
            "   j       2f             \n "
            "1:                        \n "
            "   beqz    $a2,10f        \n "
            "   addiu   $a2,$a2,-1     \n "
            "2:                        \n "
            "   sb      $a3,0($a0)     \n "
            "   addiu   $a0,$a0,1      \n "
            "   andi    $v1,$a0,0x3    \n "
            "   bnez    $v1,1b         \n "
            "3:                        \n "
            "   sltiu   $v1,$a2,4      \n "
            "   bnez    $v1,8f         \n "
            "   andi    $v1,$a1,0xff   \n "
            "   sll     $a3,$v1,0x8    \n "
            "   or      $v1,$a3,$v1    \n "
            "   sll     $a3,$v1,0x10   \n "
            "   sltiu   $a4,$a2,16     \n "
            "   or      $a3,$a3,$v1    \n "
            "   move    $a5,$a0        \n "
            "   bnez    $a4,5f         \n "
            "   move    $v1,$a0        \n "
            "   move    $a4,$a2        \n "

            // the following is the critical loop -- default non-xloops
            // version
            //"4:                        \n "
            //"   addiu   $a4,$a4,-16    \n "
            //"   sltiu   $a5,$a4,16     \n "
            //"   sw      $a3,0($v1)     \n "
            //"   sw      $a3,4($v1)     \n "
            //"   sw      $a3,8($v1)     \n "
            //"   sw      $a3,12($v1)    \n "
            //"   addiu   $v1,$v1,16     \n "
            //"   beqz    $a5,4b         \n "

            // a4 is the loop bound, we will shift the original val right
            // by 4
            // a5 is the induction variable
            "   srl     $a4, $a4, 4    \n "
            "   li      $a5, 0         \n "
            "   j       foru           \n "
            "cp_loop:                  \n "
            "   addiu   $a5,$a5,1      \n "
            "   sw      $a3,0($v1)     \n "
            "   sw      $a3,4($v1)     \n "
            "   sw      $a3,8($v1)     \n "
            "   sw      $a3,12($v1)    \n "
            "   addiu.xi $v1,$v1,16    \n "
            "foru:                     \n "
            "   for.u   $a5,$a4,cp_loop  \n "

            "   addiu   $a2,$a2,-16    \n "
            "   srl     $a5,$a2,0x4    \n "
            "   addiu   $a5,$a5,1      \n "
            "   andi    $a2,$a2,0xf    \n "
            "   sll     $a5,$a5,0x4    \n "
            "   sltiu   $v1,$a2,4      \n "
            "   addu    $a5,$a0,$a5    \n "
            "   bnez    $v1,7f         \n "
            "5:                        \n "
            "   move    $a0,$a5        \n "
            "   move    $v1,$a2        \n "
            "6:                        \n "
            "   addiu   $v1,$v1,-4     \n "
            "   sltiu   $a4,$v1,4      \n "
            "   sw      $a3,0($a0)     \n "
            "   addiu   $a0,$a0,4      \n "
            "   beqz    $a4,6b         \n "
            "   addiu   $a2,$a2,-4     \n "
            "   srl     $v1,$a2,0x2    \n "
            "   addiu   $v1,$v1,1      \n "
            "   sll     $v1,$v1,0x2    \n "
            "   addu    $a5,$a5,$v1    \n "
            "   andi    $a2,$a2,0x3    \n "
            "7:                        \n "
            "   move    $a0,$a5        \n "
            "8:                        \n "
            "   beqz    $a2,10f        \n "
            "   sll     $a1,$a1,0x18   \n "
            "   sra     $a1,$a1,0x18   \n "
            "9:                        \n "
            "   addiu   $a2,$a2,-1     \n "
            "   sb      $a1,0($a0)     \n "
            "   addiu   $a0,$a0,1      \n "
            "   bnez    $a2,9b         \n "
            "10:                       \n "
            "   jr      $ra            \n "
                : "=r" (ptr)
                : "r" (ptr), "r" (value), "r" (num)
                : "memory"

             );
  return ptr;
  #else
  return std::memset( ptr, value, num );
  #endif
}

#endif /*XLOOPS_COMMON_MEMSET_H*/

