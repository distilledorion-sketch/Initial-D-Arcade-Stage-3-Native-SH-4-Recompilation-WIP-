"""
SH4Recomp -- static SH-4 to C++ translator. See codegen.py for the pass
that turns decoded instructions into actual C++ statements.
Coverage prioritized by real instruction frequency measured across the
full 494,654-instruction disassembly of the actual game binary.
"""

def decode(op: int):
    top = (op >> 12) & 0xF
    n = (op >> 8) & 0xF
    m = (op >> 4) & 0xF
    d4 = op & 0xF
    d8 = op & 0xFF
    d12 = op & 0xFFF
    i8 = op & 0xFF
    simm8 = i8 - 0x100 if i8 & 0x80 else i8
    low = op & 0xF
    sub8 = op & 0xFF

    if op == 0x0009: return ("nop", {})
    if op == 0x000B: return ("rts", {})
    if op == 0x0028: return ("clrmac", {})

    # ---- mov.l family (35% of all instructions -- covered broadly) ----
    if top == 0xE: return ("movi", {"n": n, "imm": simm8})
    if top == 0x6 and low == 0x3: return ("mov", {"n": n, "m": m})
    if top == 0x6 and low == 0x2: return ("movl_ld", {"n": n, "m": m})          # @Rm
    if top == 0x2 and low == 0x2: return ("movl_st", {"n": n, "m": m})          # @Rn
    if top == 0x2 and low == 0x6: return ("movl_st_predec", {"n": n, "m": m})   # @-Rn
    if top == 0x6 and low == 0x6: return ("movl_ld_postinc", {"n": n, "m": m})  # @Rm+
    if top == 0x1: return ("movl_st_disp", {"n": n, "m": m, "disp": d4 * 4})    # @(disp,Rn)
    if top == 0x5: return ("movl_ld_disp", {"n": n, "m": m, "disp": d4 * 4})    # @(disp,Rm)
    # SH-4: 0000nnnnmmmm0110 = MOV.L Rm,@(R0,Rn)  -> STORE
    #       0000nnnnmmmm1110 = MOV.L @(R0,Rm),Rn  -> LOAD
    # These two were REVERSED here, which mistranslated every R0-indexed 32-bit
    # store as a load (and vice versa). Confirmed at 0x0C0220C2 (opcode 0x0E46 =
    # MOV.L R4,@(R0,R14)), where the bogus load clobbered R14 and collapsed SP.
    # The .B/.W variants below were already correct; only the .L pair was swapped.
    if top == 0x0 and low == 0x6: return ("movl_st_r0", {"n": n, "m": m})       # Rm -> @(R0,Rn)
    if top == 0x0 and low == 0xE: return ("movl_ld_r0", {"n": n, "m": m})       # @(R0,Rm) -> Rn
    # NOTE: 0x3nm4 is DIV1 Rm,Rn (0011nnnnmmmm0100), NOT a mov -- the real
    # mov.l Rm,@-Rn is 0x2nm6. The old spurious "movl_st_predec_alt" rule here
    # shadowed the correct div1 rule below and had no codegen handler. Removed so
    # 0x3nm4 decodes as div1.
    if top == 0xD: return ("movl_pcrel", {"n": n, "disp": d8 * 4})
    if sub8_gbr := (top == 0x1 and False): pass
    if op & 0xFF00 == 0x1F00: pass  # placeholder, unused

    # mov.b / mov.w families
    if top == 0x2 and low == 0x0: return ("movb_st", {"n": n, "m": m})
    if top == 0x6 and low == 0x0: return ("movb_ld", {"n": n, "m": m})
    if top == 0x2 and low == 0x4: return ("movb_st_predec", {"n": n, "m": m})
    if top == 0x6 and low == 0x4: return ("movb_ld_postinc", {"n": n, "m": m})
    if top == 0x8 and n == 0x0: return ("movb_st_disp", {"m": m, "disp": d4})       # @(disp,Rn) n from low nibble field differs; handled in codegen via m
    if top == 0x8 and n == 0x4: return ("movb_ld_disp", {"m": m, "disp": d4})
    if top == 0x2 and low == 0x1: return ("movw_st", {"n": n, "m": m})
    if top == 0x6 and low == 0x1: return ("movw_ld", {"n": n, "m": m})
    if top == 0x2 and low == 0x5: return ("movw_st_predec", {"n": n, "m": m})
    if top == 0x6 and low == 0x5: return ("movw_ld_postinc", {"n": n, "m": m})
    if top == 0x8 and n == 0x1: return ("movw_st_disp", {"m": m, "disp": d4 * 2})
    if top == 0x8 and n == 0x5: return ("movw_ld_disp", {"m": m, "disp": d4 * 2})
    if top == 0x9: return ("movw_pcrel", {"n": n, "disp": d8 * 2})
    if top == 0x0 and low == 0x5: return ("movw_st_r0", {"n": n, "m": m})
    if top == 0x0 and low == 0xD: return ("movw_ld_r0", {"n": n, "m": m})
    if top == 0x0 and low == 0x4: return ("movb_st_r0", {"n": n, "m": m})
    if top == 0x0 and low == 0xC: return ("movb_ld_r0", {"n": n, "m": m})

    # GBR-relative
    if top == 0xC and n == 0x0: return ("movb_st_gbr", {"disp": d8})
    if top == 0xC and n == 0x1: return ("movw_st_gbr", {"disp": d8 * 2})
    if top == 0xC and n == 0x2: return ("movl_st_gbr", {"disp": d8 * 4})
    if top == 0xC and n == 0x4: return ("movb_ld_gbr", {"disp": d8})
    if top == 0xC and n == 0x5: return ("movw_ld_gbr", {"disp": d8 * 2})
    if top == 0xC and n == 0x6: return ("movl_ld_gbr", {"disp": d8 * 4})
    if top == 0xC and n == 0x3: return ("trapa", {"imm": d8})
    if top == 0xC and n == 0x7: return ("mova", {"disp": d8 * 4})
    if top == 0xC and n == 0x9: return ("and_imm", {"imm": i8})
    if top == 0xC and n == 0x8: return ("tst_imm", {"imm": i8})
    if top == 0xC and n == 0xB: return ("or_imm", {"imm": i8})
    if top == 0xC and n == 0xA: return ("xor_imm", {"imm": i8})

    # ---- arithmetic / logic ----
    if top == 0x7: return ("add_imm", {"n": n, "imm": simm8})
    if top == 0x3 and low == 0xC: return ("add", {"n": n, "m": m})
    if top == 0x3 and low == 0xE: return ("addc", {"n": n, "m": m})
    if top == 0x3 and low == 0xF: return ("addv", {"n": n, "m": m})
    if top == 0x3 and low == 0x8: return ("sub", {"n": n, "m": m})
    if top == 0x3 and low == 0xA: return ("subc", {"n": n, "m": m})
    if top == 0x3 and low == 0x0: return ("cmp_eq", {"n": n, "m": m})
    if top == 0x3 and low == 0x2: return ("cmp_hs", {"n": n, "m": m})
    if top == 0x3 and low == 0x3: return ("cmp_ge", {"n": n, "m": m})
    if top == 0x3 and low == 0x6: return ("cmp_hi", {"n": n, "m": m})
    if top == 0x2 and low == 0xC: return ("cmp_str", {"n": n, "m": m})
    if top == 0x3 and low == 0x7: return ("cmp_gt", {"n": n, "m": m})
    if top == 0x2 and low == 0x9: return ("and", {"n": n, "m": m})
    if top == 0x2 and low == 0xB: return ("or", {"n": n, "m": m})
    if top == 0x2 and low == 0xA: return ("xor", {"n": n, "m": m})
    if top == 0x8 and n == 0x8: return ("cmp_eq_imm", {"imm": simm8})
    if top == 0x2 and low == 0x8: return ("tst", {"n": n, "m": m})
    if top == 0x2 and low == 0xF: return ("muls", {"n": n, "m": m})
    if top == 0x2 and low == 0xE: return ("mulu", {"n": n, "m": m})
    if top == 0x0 and low == 0x7 and (op & 0xF00F) == 0x0007: return ("mull", {"n": n, "m": m})
    if top == 0x6 and low == 0x7: return ("not", {"n": n, "m": m})
    if top == 0x6 and low == 0x8: return ("swapb", {"n": n, "m": m})
    if top == 0x6 and low == 0x9: return ("swapw", {"n": n, "m": m})
    if top == 0x6 and low == 0xC: return ("extub", {"n": n, "m": m})
    if top == 0x6 and low == 0xD: return ("extuw", {"n": n, "m": m})
    if top == 0x6 and low == 0xE: return ("extsb", {"n": n, "m": m})
    if top == 0x6 and low == 0xF: return ("extsw", {"n": n, "m": m})
    if top == 0x6 and low == 0xA: return ("negc", {"n": n, "m": m})
    if top == 0x6 and low == 0xB: return ("neg", {"n": n, "m": m})

    # shifts (register-count and immediate-count forms)
    if (op & 0xF0FF) == 0x4000: return ("shll", {"n": n})
    if (op & 0xF0FF) == 0x4001: return ("shlr", {"n": n})
    if (op & 0xF0FF) == 0x4008: return ("shll2", {"n": n})
    if (op & 0xF0FF) == 0x4018: return ("shll8", {"n": n})
    if (op & 0xF0FF) == 0x4028: return ("shll16", {"n": n})
    if (op & 0xF0FF) == 0x4009: return ("shlr2", {"n": n})
    if (op & 0xF0FF) == 0x4019: return ("shlr8", {"n": n})
    if (op & 0xF0FF) == 0x4029: return ("shlr16", {"n": n})
    if (op & 0xF0FF) == 0x4020: return ("shal", {"n": n})
    if (op & 0xF0FF) == 0x4021: return ("shar", {"n": n})

    # ---- control-register moves ----
    if (op & 0xF0FF) == 0x0002: return ("stc_sr", {"n": n})
    if (op & 0xF0FF) == 0x0022: return ("stc_vbr", {"n": n})
    if (op & 0xF0FF) == 0x0093: return ("ocbi", {"n": n})
    if (op & 0xF0FF) == 0x00A3: return ("ocbp", {"n": n})
    if (op & 0xF0FF) == 0x00B3: return ("ocbwb", {"n": n})
    if top == 0x2 and low == 0xD: return ("xtrct", {"n": n, "m": m})
    if (op & 0xF0FF) == 0x002A: return ("stspr", {"n": n})
    if (op & 0xF0FF) == 0x4022: return ("stsl_pr", {"n": n})
    if (op & 0xF0FF) == 0x4002: return ("stsl_mach", {"n": n})
    if (op & 0xF0FF) == 0x4012: return ("stsl_macl", {"n": n})
    if (op & 0xF0FF) == 0x4026: return ("lds_mem_pr", {"n": n})
    if (op & 0xF0FF) == 0x0029: return ("movt", {"n": n})
    if (op & 0xF0FF) == 0x000A: return ("sts_mach", {"n": n})
    if (op & 0xF0FF) == 0x001A: return ("sts_macl", {"n": n})
    if (op & 0xF0FF) == 0x006A: return ("sts_fpscr", {"n": n})
    if (op & 0xF0FF) == 0x005A: return ("sts_fpul", {"n": n})
    if (op & 0xF0FF) == 0x406A: return ("lds_fpscr", {"n": n})
    if (op & 0xF0FF) == 0x405A: return ("lds_fpul", {"n": n})
    if (op & 0xF0FF) == 0x4066: return ("lds_mem_fpscr", {"n": n})
    if (op & 0xF0FF) == 0x4056: return ("lds_mem_fpul", {"n": n})
    if (op & 0xF0FF) == 0x4062: return ("stsl_fpscr", {"n": n})
    if (op & 0xF00F) == 0x3005: return ("dmulu", {"n": n, "m": m})
    if (op & 0xF0FF) == 0x4024: return ("rotcl", {"n": n})
    if (op & 0xF0FF) == 0x4025: return ("rotcr", {"n": n})
    if (op & 0xF0FF) == 0x4004: return ("rotl", {"n": n})
    if (op & 0xF0FF) == 0x4005: return ("rotr", {"n": n})
    # SH-4 special FPU family.  These share low nibble 0xD but are
    # distinct opcodes; keep the masks aligned with Flycast's SH-4 table.
    if (op & 0xF0FF) == 0xF00D: return ("fsts_fpul", {"n": n})
    if (op & 0xF0FF) == 0xF01D: return ("flds_fpul", {"n": n})
    if (op & 0xF0FF) == 0xF02D: return ("float_fpul", {"n": n})
    if (op & 0xF0FF) == 0xF03D: return ("ftrc_fpul", {"n": n})
    if (op & 0xF0FF) == 0xF04D: return ("fneg_special", {"n": n})
    if (op & 0xF0FF) == 0xF05D: return ("fabs_special", {"n": n})
    if (op & 0xF0FF) == 0xF06D: return ("fsqrt_special", {"n": n})
    if (op & 0xF0FF) == 0xF07D: return ("fsrra", {"n": n})
    if (op & 0xF0FF) == 0xF08D: return ("fldi0_special", {"n": n})
    if (op & 0xF0FF) == 0xF09D: return ("fldi1_special", {"n": n})
    if (op & 0xF0FF) == 0xF0AD: return ("fcnvsd", {"n": n})
    if (op & 0xF0FF) == 0xF0BD: return ("fcnvds", {"n": n})
    if (op & 0xF0FF) == 0xF0ED: return ("fipr", {"n": n & 0xC, "m": (n & 0x3) << 2})
    if (op & 0xF1FF) == 0xF0FD: return ("fsca", {"n": n & 0xE})
    if op == 0xF3FD: return ("fschg", {})
    if op == 0xFBFD: return ("frchg", {})
    if (op & 0xF3FF) == 0xF1FD: return ("ftrv", {"n": n & 0xC})
    if (op & 0xF0FF) == 0x400A: return ("lds_mach", {"n": n})
    if (op & 0xF0FF) == 0x401A: return ("lds_macl", {"n": n})
    if (op & 0xF0FF) == 0x4006: return ("ldsl_mach", {"n": n})
    if (op & 0xF0FF) == 0x4016: return ("ldsl_macl", {"n": n})
    if (op & 0xF0FF) == 0x0012: return ("stcgbr", {"n": n})
    if (op & 0xF0FF) == 0x400E: return ("ldcsr", {"n": n})
    if (op & 0xF0FF) == 0x401E: return ("ldcgbr", {"n": n})
    if (op & 0xF0FF) == 0x4007: return ("ldcl_sr", {"n": n})
    if (op & 0xF0FF) == 0x4010: return ("dt", {"n": n})
    if (op & 0xF0FF) == 0x4015: return ("cmp_pl", {"n": n})
    if (op & 0xF0FF) == 0x4011: return ("cmp_pz", {"n": n})
    if op == 0x0008: return ("clrt", {})
    if op == 0x0018: return ("sett", {})
    if op == 0x0019: return ("div0u", {})
    if top == 0x2 and low == 0x7: return ("div0s", {"n": n, "m": m})
    if top == 0x3 and low == 0x4: return ("div1", {"n": n, "m": m})
    if (op & 0xF00F) == 0x400C: return ("shad", {"n": n, "m": m})
    if (op & 0xF00F) == 0x400D: return ("shld", {"n": n, "m": m})
    if (op & 0xF0FF) == 0x0083 or (op & 0xF000FF) == 0x83: pass
    if (op & 0xF0FF) in (0x0083,): return ("pref", {"n": n})

    # ---- floating point (extended) ----
    if (op & 0xF00F) == 0x300 and top==0xF and low==0x0: pass

    # ---- floating point (fmov.s/fmov ~5% combined) ----
    if top == 0xF and low == 0xC: return ("fmov", {"n": n, "m": m})
    if top == 0xF and low == 0x8: return ("fmovs_ld", {"n": n, "m": m})
    if top == 0xF and low == 0xA: return ("fmovs_st", {"n": n, "m": m})
    if top == 0xF and low == 0x9: return ("fmovs_ld_postinc", {"n": n, "m": m})
    if top == 0xF and low == 0xB: return ("fmovs_st_predec", {"n": n, "m": m})
    if top == 0xF and low == 0x6: return ("fmovs_ld_r0", {"n": n, "m": m})
    if top == 0xF and low == 0x7: return ("fmovs_st_r0", {"n": n, "m": m})
    if top == 0xF and low == 0x0: return ("fadd", {"n": n, "m": m})
    if top == 0xF and low == 0x1: return ("fsub", {"n": n, "m": m})
    if top == 0xF and low == 0x2: return ("fmul", {"n": n, "m": m})
    if top == 0xF and low == 0x3: return ("fdiv", {"n": n, "m": m})
    if top == 0xF and low == 0x4: return ("fcmp_eq", {"n": n, "m": m})
    if top == 0xF and low == 0x5: return ("fcmp_gt", {"n": n, "m": m})
    if top == 0xF and low == 0xE: return ("fmac", {"n": n, "m": m})
    if (op & 0xF0FF) == 0xF08D or (op & 0xF0FF) == 0xF06D: pass

    # ---- branches / control flow ----
    if top == 0x8 and n == 0xB: return ("bf", {"disp": simm8 * 2 + 4})
    if top == 0x8 and n == 0x9: return ("bt", {"disp": simm8 * 2 + 4})
    if top == 0x8 and n == 0xF: return ("bf_s", {"disp": simm8 * 2 + 4})
    if top == 0x8 and n == 0xD: return ("bt_s", {"disp": simm8 * 2 + 4})
    if top == 0xA:
        disp = d12 - 0x1000 if d12 & 0x800 else d12
        return ("bra", {"disp": disp * 2 + 4})
    if top == 0xB:
        disp = d12 - 0x1000 if d12 & 0x800 else d12
        return ("bsr", {"disp": disp * 2 + 4})
    if (op & 0xF0FF) == 0x0003: return ("bsrf", {"n": n})
    if (op & 0xF0FF) == 0x0023: return ("braf", {"n": n})
    if (op & 0xF0FF) == 0x400B: return ("jsr", {"n": n})
    if (op & 0xF0FF) == 0x402B: return ("jmp", {"n": n})

    return ("unk", {"raw": op})
