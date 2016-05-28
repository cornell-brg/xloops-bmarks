	.section .mdebug.eabi32
	.section .gcc_compiled_long32
	.previous
	.file	"dither-xloops-mem.ll"
	.text
	.globl	_ZN6dither17dither_xloops_memEPhS0_ii
	.align	2
	.type	_ZN6dither17dither_xloops_memEPhS0_ii,@function
	.set	nomips16                # @_ZN6dither17dither_xloops_memEPhS0_ii
	.ent	_ZN6dither17dither_xloops_memEPhS0_ii
_ZN6dither17dither_xloops_memEPhS0_ii:
	.cfi_startproc
	.frame	$fp,48,$ra
	.mask 	0xc0ff0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
# BB#0:                                 # %entry
	addiu	$sp, $sp, -48
$tmp3:
	.cfi_def_cfa_offset 48
	sw	$ra, 44($sp)            # 4-byte Folded Spill
	sw	$fp, 40($sp)            # 4-byte Folded Spill
	sw	$23, 36($sp)            # 4-byte Folded Spill
	sw	$22, 32($sp)            # 4-byte Folded Spill
	sw	$21, 28($sp)            # 4-byte Folded Spill
	sw	$20, 24($sp)            # 4-byte Folded Spill
	sw	$19, 20($sp)            # 4-byte Folded Spill
	sw	$18, 16($sp)            # 4-byte Folded Spill
	sw	$17, 12($sp)            # 4-byte Folded Spill
	sw	$16, 8($sp)             # 4-byte Folded Spill
$tmp4:
	.cfi_offset 31, -4
$tmp5:
	.cfi_offset 30, -8
$tmp6:
	.cfi_offset 23, -12
$tmp7:
	.cfi_offset 22, -16
$tmp8:
	.cfi_offset 21, -20
$tmp9:
	.cfi_offset 20, -24
$tmp10:
	.cfi_offset 19, -28
$tmp11:
	.cfi_offset 18, -32
$tmp12:
	.cfi_offset 17, -36
$tmp13:
	.cfi_offset 16, -40
	addu	$fp, $sp, $zero
$tmp14:
	.cfi_def_cfa_register 30
	addu	$16, $zero, $7
	addu	$17, $zero, $6
	addu	$18, $zero, $5
	addu	$19, $zero, $4
	addiu	$2, $zero, -8
	addiu	$21, $16, 2
	addiu	$1, $17, 1
	mul	$1, $21, $1
	sll	$1, $1, 2
	addiu	$1, $1, 7
	and	$1, $1, $2
	addiu	$sp, $sp, 0
	subu	$22, $sp, $1
	addu	$sp, $zero, $22
	slti	$1, $17, 1
	addiu	$sp, $sp, 0
	sw	$1, 4($fp)              # 4-byte Folded Spill
	bne	$1, $zero, $BB0_11
# BB#1:                                 # %for.cond2.preheader.lr.ph
	slti	$1, $21, 1
	bne	$1, $zero, $BB0_5
# BB#2:                                 # %for.cond2.preheader.lr.ph.split.us
	sll	$1, $16, 2
	addiu	$23, $zero, 0
	addiu	$20, $1, 8
$BB0_3:                                 # %for.inc7.us
                                        # =>This Inner Loop Header: Depth=1
	mul	$1, $20, $23
	addu	$4, $22, $1
	addiu	$sp, $sp, 0
	addiu	$5, $zero, 0
	addu	$6, $zero, $20
	jal	memset
	addiu	$sp, $sp, 0
	addiu	$23, $23, 1
	bne	$23, $17, $BB0_3
# BB#4:                                 # %for.cond10.preheader
	lw	$1, 4($fp)              # 4-byte Folded Reload
	bne	$1, $zero, $BB0_11
$BB0_5:                                 # %for.cond13.preheader.lr.ph
	addiu	$2, $zero, 0
	blez	$16, $BB0_11
  j $BB_10
$BB0_6:                                 # %for.body15.lr.ph.us
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_7 Depth 2
	mul	$5, $2, $21
	mul	$4, $2, $16
	addiu	$2, $2, 1
	addiu	$3, $zero, 0
	addiu	$9, $zero, 7
	addiu	$10, $zero, 5
	addiu	$11, $zero, 3
	addiu	$12, $zero, 127
	addiu	$25, $zero, 0
	mul	$6, $2, $21
	addiu	$7, $5, 1
	addiu	$8, $5, 2
$BB0_7:                                 # %for.body15.us
                                        #   Parent Loop BB0_6 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$13, $3, $4
	addiu	$15, $zero, 0
	addiu	$24, $zero, 0
	addu	$1, $18, $13
	lbu	$14, 0($1)
	beq	$14, $zero, $BB0_9
# BB#8:                                 # %if.then.us
                                        #   in Loop: Header=BB0_7 Depth=2
	mul	$1, $25, $9
	addu	$15, $8, $3
	addiu	$24, $zero, -1
	sll	$15, $15, 2
	addu	$15, $22, $15
	lw	$15, 0($15)
	addu	$1, $15, $1
	addu	$15, $7, $3
	sll	$15, $15, 2
	addu	$15, $22, $15
	lw	$15, 0($15)
	mul	$15, $15, $10
	addu	$1, $1, $15
	addu	$15, $3, $5
	sll	$15, $15, 2
	addu	$15, $22, $15
	lw	$15, 0($15)
	mul	$15, $15, $11
	addu	$1, $1, $15
	sra	$15, $1, 31
	srl	$15, $15, 28
	addu	$1, $1, $15
	sra	$1, $1, 4
	addu	$1, $1, $14
	slt	$14, $12, $1
	movz	$24, $zero, $14
	andi	$14, $24, 255
	subu	$15, $1, $14
$BB0_9:                                 # %if.end40.us
                                        #   in Loop: Header=BB0_7 Depth=2
	addiu	$3, $3, 1
	addu	$25, $zero, $15
	addu	$1, $3, $6
	sll	$1, $1, 2
	addu	$1, $22, $1
	sw	$15, 0($1)
	addu	$1, $19, $13
	sb	$24, 0($1)
	bne	$3, $16, $BB0_7
$BB_10:                                # %for.inc49.us
                                        #   in Loop: Header=BB0_6 Depth=1
	for.m	$2, $17, $BB0_6
$BB0_11:                                # %for.end51
	addu	$sp, $fp, $zero
	lw	$16, 8($sp)             # 4-byte Folded Reload
	lw	$17, 12($sp)            # 4-byte Folded Reload
	lw	$18, 16($sp)            # 4-byte Folded Reload
	lw	$19, 20($sp)            # 4-byte Folded Reload
	lw	$20, 24($sp)            # 4-byte Folded Reload
	lw	$21, 28($sp)            # 4-byte Folded Reload
	lw	$22, 32($sp)            # 4-byte Folded Reload
	lw	$23, 36($sp)            # 4-byte Folded Reload
	lw	$fp, 40($sp)            # 4-byte Folded Reload
	lw	$ra, 44($sp)            # 4-byte Folded Reload
	addiu	$sp, $sp, 48
	jr	$ra
	.set	at
	.set	macro
	.set	reorder
	.end	_ZN6dither17dither_xloops_memEPhS0_ii
$tmp15:
	.size	_ZN6dither17dither_xloops_memEPhS0_ii, ($tmp15)-_ZN6dither17dither_xloops_memEPhS0_ii
	.cfi_endproc


