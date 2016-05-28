	.section .mdebug.eabi32
	.section .gcc_compiled_long32
	.previous
	.file	"../mibench-adpcmdec/mibench-adpcmdec-xloops-opt.cc"
	.text
	.globl	_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE
	.align	2
	.type	_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE,@function
	.set	nomips16
	.ent	_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE
_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE:
	.cfi_startproc
	.frame	$sp,0,$ra
	.mask 	0x00000000,0
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	.set	noat
  addiu $29, $29, -8
  sw    $16, 4($29)
	slti	$1, $6, 1
	bne	$1, $zero, $BB0_9
	lb	$2, 2($7)
	lui	$3, %hi(_ZN16mibench_adpcmdecL13stepsizeTableE)
	lh	$11, 0($7)
	addiu	$9, $zero, 88
	addiu	$10, $zero, -32768
	addiu	$12, $zero, 0
	addiu	$8, $3, %lo(_ZN16mibench_adpcmdecL13stepsizeTableE)
	sll	$1, $2, 1
	addu	$1, $8, $1
	addiu	$8, $zero, 0
	lh	$13, 0($1)
	lui	$1, %hi(_ZN16mibench_adpcmdecL10indexTableE)
	addiu	$7, $1, %lo(_ZN16mibench_adpcmdecL10indexTableE)
	j	$BB0_10

$BB0_2:                                 # %for.body
                                        # =>This Inner Loop Header: Depth=1
	andi	$15, $12, 1    # uses   bufferstep
	xori  $12, $12, 1    # update bufferstep
	andi	$12, $12, 1    # update bufferstep
	srl	  $1,  $8, 31    # |
	addu	$1,  $8, $1    # |
	sra	  $1,  $1,  1    # |
	addu	$1,  $4, $1    # |
	lb   	$1,  0($1)     # | indata[i/2]
	sra	  $14, $1, 4     # indata[i/2] >> 4
	movn	$14, $1, $15   # inputbuffer update
	sra	  $15, $13, 3    # vpdiff = step >> 3
	andi	$1,  $14, 15   # delta = inputbuffer & 0xf
	sll	  $1,  $1, 2     # convert delta to an index
	addu	$1,  $7, $1    # addr for indexTable[delta]
	lw	  $1,  0($1)     # load indexTable[delta]
	addu	$2,  $1, $2    # index += indexTable[delta]
	slti	$1,  $2, 0     # index < INDEX_MIN
	movn	$2,  $zero, $1 # index update
	slt	  $25, $9,    $2 # index > INDEX_MAX
	movn	$2,  $9,   $25 # index = INDEX_MAX or not
	sll	  $1,  $14, 29   # calc using inputbuffer/delta
	sra	  $1,  $1, 31    # calc using inputbuffer/delta
	and	  $1,  $1, $13   # calc using inputbuffer/delta
	addu	$15, $1, $15   # vpdiff += update
	andi	$1,  $14, 2    # delta & 2 ?
	beq	  $1,  $zero, $BB0_4
# BB#3:                                 # %if.then27
                                        #   in Loop: Header=BB0_2 Depth=1
	sra	  $1,  $13, 1    # step >> 1
	addu	$15, $15, $1   # vpdiff += step >>1
$BB0_4:                                 # %if.end30
                                        #   in Loop: Header=BB0_2 Depth=1
	andi	$24, $14,   8  # delta & 8 --> sign ?
	andi	$1,  $14,   1  # delta & 1
	beq	  $1,  $zero, $BB0_6
# BB#5:                                 # %if.then33
                                        #   in Loop: Header=BB0_2 Depth=1
	sra	  $1,  $13,  2   # step >> 2
	addu	$15, $15, $1   # vpdiff update
$BB0_6:                                 # %if.end36
                                        #   in Loop: Header=BB0_2 Depth=1
	addiu	$13, $3,  %lo(_ZN16mibench_adpcmdecL13stepsizeTableE) # step = tb[index]
	sll	  $1,  $2,  1       # addr calc of index
	addu	$1,  $13, $1      # addr calc of index
	lh	  $13, 0($1)        # step = tb[index]
	subu	$1,  $zero,$15    # -vpdiff
	movz	$1,  $15,  $24    # if ( !sign ) $1 = vpdiff
	addu	$16, $1,   $11    # valpred update, use a different name for step here
	addiu	$11, $zero, 32767 # move VALPRED_MAX
	slt	  $1,  $11,  $16    # valpred > VALPRED_MAX
	bne	  $1,  $zero, $BB0_8
# BB#7:                                 # %if.else44
                                        #   in Loop: Header=BB0_2 Depth=1
	slti	$1, $16, -32768   # valpred < VALPRED_MIN
	movn	$16, $10, $1      # valpred = VALPRED_MIN or has the update
	addu	$11, $zero, $16   # $11 has valpred
$BB0_8:                                 # %if.end48
                                        #   in Loop: Header=BB0_2 Depth=1
	addiu $11, $11, 0       # merge for valpred
	addiu	$8, $8, 1         # iv bump
	sh	$11, 0($5)          # outdata[i]
  addiu.xi $5, $5, 2      # address bump
$BB0_10:                                # %if.end48
                                        #   in Loop: Header=BB0_2 Depth=1
	for.r 	$8, $6, $BB0_2
$BB0_9:                                 # %for.end
  lw $16, 4($29)
  addiu $29, $29, 8
	jr	$ra

	.set	at
	.set	macro
	.set	reorder
	.end	_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE
$tmp0:
	.size	_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE, ($tmp0)-_ZN16mibench_adpcmdec27mibench_adpcmdec_xloops_optEPcPsiPNS_11adpcm_stateE
	.cfi_endproc

	.type	_ZN16mibench_adpcmdecL13stepsizeTableE,@object
	.section	.rodata,"a",@progbits
	.align	1
_ZN16mibench_adpcmdecL13stepsizeTableE:
	.2byte	7
	.2byte	8
	.2byte	9
	.2byte	10
	.2byte	11
	.2byte	12
	.2byte	13
	.2byte	14
	.2byte	16
	.2byte	17
	.2byte	19
	.2byte	21
	.2byte	23
	.2byte	25
	.2byte	28
	.2byte	31
	.2byte	34
	.2byte	37
	.2byte	41
	.2byte	45
	.2byte	50
	.2byte	55
	.2byte	60
	.2byte	66
	.2byte	73
	.2byte	80
	.2byte	88
	.2byte	97
	.2byte	107
	.2byte	118
	.2byte	130
	.2byte	143
	.2byte	157
	.2byte	173
	.2byte	190
	.2byte	209
	.2byte	230
	.2byte	253
	.2byte	279
	.2byte	307
	.2byte	337
	.2byte	371
	.2byte	408
	.2byte	449
	.2byte	494
	.2byte	544
	.2byte	598
	.2byte	658
	.2byte	724
	.2byte	796
	.2byte	876
	.2byte	963
	.2byte	1060
	.2byte	1166
	.2byte	1282
	.2byte	1411
	.2byte	1552
	.2byte	1707
	.2byte	1878
	.2byte	2066
	.2byte	2272
	.2byte	2499
	.2byte	2749
	.2byte	3024
	.2byte	3327
	.2byte	3660
	.2byte	4026
	.2byte	4428
	.2byte	4871
	.2byte	5358
	.2byte	5894
	.2byte	6484
	.2byte	7132
	.2byte	7845
	.2byte	8630
	.2byte	9493
	.2byte	10442
	.2byte	11487
	.2byte	12635
	.2byte	13899
	.2byte	15289
	.2byte	16818
	.2byte	18500
	.2byte	20350
	.2byte	22385
	.2byte	24623
	.2byte	27086
	.2byte	29794
	.2byte	32767
	.size	_ZN16mibench_adpcmdecL13stepsizeTableE, 178

	.type	_ZN16mibench_adpcmdecL10indexTableE,@object
	.align	2
_ZN16mibench_adpcmdecL10indexTableE:
	.4byte	4294967295
	.4byte	4294967295
	.4byte	4294967295
	.4byte	4294967295
	.4byte	2
	.4byte	4
	.4byte	6
	.4byte	8
	.4byte	4294967295
	.4byte	4294967295
	.4byte	4294967295
	.4byte	4294967295
	.4byte	2
	.4byte	4
	.4byte	6
	.4byte	8
	.size	_ZN16mibench_adpcmdecL10indexTableE, 64


