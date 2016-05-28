	.section .mdebug.eabi32
	.section .gcc_compiled_long32
	.previous
	.file	"dither-xloops-reg-opt.ll"
	.text
	.globl	_ZN6dither21dither_xloops_reg_optEPhS0_ii
	.align	2
	.type	_ZN6dither21dither_xloops_reg_optEPhS0_ii,@function
	.set	nomips16                # @_ZN6dither21dither_xloops_reg_optEPhS0_ii
	.ent	_ZN6dither21dither_xloops_reg_optEPhS0_ii
_ZN6dither21dither_xloops_reg_optEPhS0_ii:
	.cfi_startproc
	.frame	$fp,40,$ra
	.mask 	0xc07f0000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
# BB#0:                                 # %entry
	addiu	$sp, $sp, -40
$tmp3:
	.cfi_def_cfa_offset 40
	sw	$ra, 36($sp)            # 4-byte Folded Spill
	sw	$fp, 32($sp)            # 4-byte Folded Spill
	sw	$22, 28($sp)            # 4-byte Folded Spill
	sw	$21, 24($sp)            # 4-byte Folded Spill
	sw	$20, 20($sp)            # 4-byte Folded Spill
	sw	$19, 16($sp)            # 4-byte Folded Spill
	sw	$18, 12($sp)            # 4-byte Folded Spill
	sw	$17, 8($sp)             # 4-byte Folded Spill
	sw	$16, 4($sp)             # 4-byte Folded Spill
$tmp4:
	.cfi_offset 31, -4
$tmp5:
	.cfi_offset 30, -8
$tmp6:
	.cfi_offset 22, -12
$tmp7:
	.cfi_offset 21, -16
$tmp8:
	.cfi_offset 20, -20
$tmp9:
	.cfi_offset 19, -24
$tmp10:
	.cfi_offset 18, -28
$tmp11:
	.cfi_offset 17, -32
$tmp12:
	.cfi_offset 16, -36
	addu	$fp, $sp, $zero
$tmp13:
	.cfi_def_cfa_register 30
	addu	$16, $zero, $7
	addu	$17, $zero, $6
	addu	$18, $zero, $5
	addu	$19, $zero, $4
	addiu	$3, $zero, -8
	addiu	$1, $16, 2
	sll	$2, $1, 2
	addiu	$2, $2, 7
	and	$2, $2, $3
	addiu	$sp, $sp, 0
	subu	$21, $sp, $2
	addu	$sp, $zero, $21
	addiu	$sp, $sp, 0
	addiu	$sp, $sp, 0
	subu	$20, $sp, $2
	addu	$sp, $zero, $20
	slti	$1, $1, 1
	addiu	$sp, $sp, 0
	bne	$1, $zero, $BB0_2
# BB#1:                                 # %for.body.lr.ph
	addiu	$sp, $sp, 0
	sll	$1, $16, 2
	addu	$4, $zero, $21
	addiu	$5, $zero, 0
	addiu	$22, $1, 8
	addu	$6, $zero, $22
	jal	memset
	addiu	$sp, $sp, 0
	addiu	$sp, $sp, 0
	addu	$4, $zero, $20
	addiu	$5, $zero, 0
	addu	$6, $zero, $22
	jal	memset
	addiu	$sp, $sp, 0
$BB0_2:                                 # %for.cond7.preheader
	slti	$1, $17, 1
	bne	$1, $zero, $BB0_10
# BB#3:                                 # %for.cond7.preheader
	blez	$16, $BB0_10
# BB#4:
	addiu	$10, $zero, 0
	addiu	$2, $zero, 7
	addiu	$3, $zero, 5
	addiu	$4, $zero, 3
	addiu	$5, $zero, 127
	addu	$6, $zero, $10
$BB0_5:                                 # %for.body12.lr.ph.us
                                        # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_6 Depth 2
	addu	$7, $zero, $20
	addu	$20, $zero, $21
	mul	$8, $6, $16
	addiu	$9, $zero, 0
	addu	$14, $zero, $10
	j	$BB0_11
$BB0_6:                                 # %for.body12.us
                                        #   Parent Loop BB0_5 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	addu	$11, $9, $8
	addiu	$13, $zero, 0
	addiu	$10, $zero, 0
	addu	$1, $18, $11
	lbu	$12, 0($1)
	beq	$12, $zero, $BB0_8
# BB#7:                                 # %if.then.us
                                        #   in Loop: Header=BB0_6 Depth=2
	sll	$1, $9, 2
	addu	$1, $20, $1
	lw	$13, 4($1)
	lw	$10, 8($1)
	lw	$1, 0($1)
	mul	$13, $13, $3
	mul	$1, $1, $4
	addu	$1, $13, $1
	mul	$13, $14, $2
	addu	$10,  $10, $13
	addiu	$13, $zero, -1
	addu	$1, $10, $1
	sra	$10, $1, 31
	srl	$10, $10, 28
	addu	$1, $1, $10
	sra	$1, $1, 4
	addu	$1, $1, $12
	slt	$10, $5, $1
	movz	$13, $zero, $10
	andi	$10, $13, 255
	subu	$10, $1, $10
$BB0_8:                                 # %if.end34.us
                                        #   in Loop: Header=BB0_6 Depth=2
	addu	$14, $zero, $10
	addiu	$9, $9, 1
	sll	$1, $9, 2
	addu	$1, $7, $1
	sw	$10, 0($1)
	addu	$1, $19, $11
	sb	$13, 0($1)
$BB0_11:                                # %if.end34.us
                                        #   in Loop: Header=BB0_6 Depth=2
	for.r 	$9, $16, $BB0_6
# BB#9:                                 # %for.end40.us
                                        #   in Loop: Header=BB0_5 Depth=1
	addiu	$6, $6, 1
	addu	$21, $zero, $7
	bne	$6, $17, $BB0_5
$BB0_10:                                # %for.end43
	addu	$sp, $fp, $zero
	lw	$16, 4($sp)             # 4-byte Folded Reload
	lw	$17, 8($sp)             # 4-byte Folded Reload
	lw	$18, 12($sp)            # 4-byte Folded Reload
	lw	$19, 16($sp)            # 4-byte Folded Reload
	lw	$20, 20($sp)            # 4-byte Folded Reload
	lw	$21, 24($sp)            # 4-byte Folded Reload
	lw	$22, 28($sp)            # 4-byte Folded Reload
	lw	$fp, 32($sp)            # 4-byte Folded Reload
	lw	$ra, 36($sp)            # 4-byte Folded Reload
	addiu	$sp, $sp, 40
	jr	$ra
	.set	at
	.set	macro
	.set	reorder
	.end	_ZN6dither21dither_xloops_reg_optEPhS0_ii
$tmp14:
	.size	_ZN6dither21dither_xloops_reg_optEPhS0_ii, ($tmp14)-_ZN6dither21dither_xloops_reg_optEPhS0_ii
	.cfi_endproc


