from sh4recomp import decode

def gen_simple(mnem, args, addr):
    n = args.get("n"); m = args.get("m")
    R = lambda i: f"ctx.r[{i}]"
    if mnem == "nop": return []
    if mnem == "movi": return [f"{R(n)} = (uint32_t)(int32_t){args['imm']};"]
    if mnem == "mov": return [f"{R(n)} = {R(m)};"]

    if mnem == "movl_ld": return [f"{R(n)} = MEM32({R(m)});"]
    if mnem == "movl_st": return [f"MEM32_WRITE({R(n)}, {R(m)});"]
    if mnem == "movl_st_predec": return [f"{R(n)} -= 4;", f"MEM32_WRITE({R(n)}, {R(m)});"]
    if mnem in ("movl_ld_postinc",):
        if n == m: return [f"{{ uint32_t _t = MEM32({R(m)}); {R(m)} += 4; {R(n)} = _t; }}"]
        return [f"{R(n)} = MEM32({R(m)});", f"{R(m)} += 4;"]
    if mnem == "movl_st_disp": return [f"MEM32_WRITE({R(n)} + {args['disp']}, {R(m)});"]
    if mnem == "movl_ld_disp": return [f"{R(n)} = MEM32({R(m)} + {args['disp']});"]
    if mnem == "movl_st_r0": return [f"MEM32_WRITE({R(n)} + ctx.r[0], {R(m)});"]
    if mnem == "movl_ld_r0": return [f"{R(n)} = MEM32({R(m)} + ctx.r[0]);"]
    if mnem == "movl_pcrel":
        target = (addr & ~3) + 4 + args["disp"]
        return [f"{R(n)} = MEM32(0x{target:08X}u);  // literal pool"]

    if mnem == "movb_ld": return [f"{R(n)} = (uint32_t)(int32_t)(int8_t)MEM8({R(m)});"]
    if mnem == "movb_st": return [f"MEM8_WRITE({R(n)}, (uint8_t){R(m)});"]
    if mnem == "movb_st_predec": return [f"{R(n)} -= 1;", f"MEM8_WRITE({R(n)}, (uint8_t){R(m)});"]
    if mnem == "movb_ld_postinc":
        if n == m: return [f"{{ uint8_t _t = MEM8({R(m)}); {R(m)} += 1; {R(n)} = (uint32_t)(int32_t)(int8_t)_t; }}"]
        return [f"{R(n)} = (uint32_t)(int32_t)(int8_t)MEM8({R(m)});", f"{R(m)} += 1;"]
    if mnem == "movb_ld_disp": return [f"ctx.r[0] = (uint32_t)(int32_t)(int8_t)MEM8({R(m)} + {args['disp']});"]
    if mnem == "movb_st_disp": return [f"MEM8_WRITE({R(m)} + {args['disp']}, (uint8_t)ctx.r[0]);"]
    if mnem == "movb_st_r0": return [f"MEM8_WRITE({R(n)} + ctx.r[0], (uint8_t){R(m)});"]
    if mnem == "movb_ld_r0": return [f"{R(n)} = (uint32_t)(int32_t)(int8_t)MEM8({R(m)} + ctx.r[0]);"]

    if mnem == "movw_ld": return [f"{R(n)} = (uint32_t)(int32_t)(int16_t)MEM16({R(m)});"]
    if mnem == "movw_st": return [f"MEM16_WRITE({R(n)}, (uint16_t){R(m)});"]
    if mnem == "movw_st_predec": return [f"{R(n)} -= 2;", f"MEM16_WRITE({R(n)}, (uint16_t){R(m)});"]
    if mnem == "movw_ld_postinc":
        if n == m: return [f"{{ uint16_t _t = MEM16({R(m)}); {R(m)} += 2; {R(n)} = (uint32_t)(int32_t)(int16_t)_t; }}"]
        return [f"{R(n)} = (uint32_t)(int32_t)(int16_t)MEM16({R(m)});", f"{R(m)} += 2;"]
    if mnem == "movw_ld_disp": return [f"ctx.r[0] = (uint32_t)(int32_t)(int16_t)MEM16({R(m)} + {args['disp']});"]
    if mnem == "movw_st_disp": return [f"MEM16_WRITE({R(m)} + {args['disp']}, (uint16_t)ctx.r[0]);"]
    if mnem == "movw_st_r0": return [f"MEM16_WRITE({R(n)} + ctx.r[0], (uint16_t){R(m)});"]
    if mnem == "movw_ld_r0": return [f"{R(n)} = (uint32_t)(int32_t)(int16_t)MEM16({R(m)} + ctx.r[0]);"]
    if mnem == "movw_pcrel":
        target = (addr & ~1) + 4 + args["disp"]
        return [f"{R(n)} = (uint32_t)(int32_t)(int16_t)MEM16(0x{target:08X}u);"]

    if mnem == "movb_ld_gbr": return [f"ctx.r[0] = (uint32_t)(int32_t)(int8_t)MEM8(ctx.gbr + {args['disp']});"]
    if mnem == "movw_ld_gbr": return [f"ctx.r[0] = (uint32_t)(int32_t)(int16_t)MEM16(ctx.gbr + {args['disp']});"]
    if mnem == "movl_ld_gbr": return [f"ctx.r[0] = MEM32(ctx.gbr + {args['disp']});"]
    if mnem == "movb_st_gbr": return [f"MEM8_WRITE(ctx.gbr + {args['disp']}, (uint8_t)ctx.r[0]);"]
    if mnem == "movw_st_gbr": return [f"MEM16_WRITE(ctx.gbr + {args['disp']}, (uint16_t)ctx.r[0]);"]
    if mnem == "movl_st_gbr": return [f"MEM32_WRITE(ctx.gbr + {args['disp']}, ctx.r[0]);"]
    if mnem == "mova":
        target = (addr & ~3) + 4 + args["disp"]
        return [f"ctx.r[0] = 0x{target:08X}u;"]
    if mnem == "trapa": return [f"host_trapa(ctx, {args['imm']});"]

    if mnem == "add_imm": return [f"{R(n)} += (int32_t){args['imm']};"]
    if mnem == "add": return [f"{R(n)} += {R(m)};"]
    if mnem == "addc": return [f"{{ uint64_t _s = (uint64_t){R(n)} + {R(m)} + (ctx.t?1:0); ctx.t = _s > 0xFFFFFFFFu; {R(n)} = (uint32_t)_s; }}"]
    if mnem == "addv": return [f"{{ int64_t _s = (int64_t)(int32_t){R(n)} + (int32_t){R(m)}; ctx.t = (_s != (int32_t)_s); {R(n)} += {R(m)}; }}"]
    if mnem == "sub": return [f"{R(n)} -= {R(m)};"]
    if mnem == "subc": return [f"{{ uint64_t _s = (uint64_t){R(n)} - {R(m)} - (ctx.t?1:0); ctx.t = {R(n)} < ({R(m)} + (ctx.t?1u:0u)); {R(n)} = (uint32_t)_s; }}"]
    if mnem == "cmp_eq": return [f"ctx.t = ({R(n)} == {R(m)});"]
    if mnem == "cmp_eq_imm": return [f"ctx.t = (ctx.r[0] == (uint32_t)(int32_t){args['imm']});"]
    if mnem == "cmp_hs": return [f"ctx.t = ({R(n)} >= {R(m)});"]
    if mnem == "cmp_hi": return [f"ctx.t = ({R(n)} > {R(m)});"]
    if mnem == "cmp_str":
        return [f"{{ uint32_t _x = {R(n)} ^ {R(m)}; "
                f"ctx.t = (((_x - 0x01010101u) & ~_x & 0x80808080u) != 0); }}"]
    if mnem == "cmp_ge": return [f"ctx.t = ((int32_t){R(n)} >= (int32_t){R(m)});"]
    if mnem == "cmp_gt": return [f"ctx.t = ((int32_t){R(n)} > (int32_t){R(m)});"]
    if mnem == "and": return [f"{R(n)} &= {R(m)};"]
    if mnem == "and_imm": return [f"ctx.r[0] &= (uint32_t){args['imm']};"]
    if mnem == "or": return [f"{R(n)} |= {R(m)};"]
    if mnem == "or_imm": return [f"ctx.r[0] |= (uint32_t){args['imm']};"]
    if mnem == "xor": return [f"{R(n)} ^= {R(m)};"]
    if mnem == "xor_imm": return [f"ctx.r[0] ^= (uint32_t){args['imm']};"]
    if mnem == "tst": return [f"ctx.t = (({R(n)} & {R(m)}) == 0);"]
    if mnem == "tst_imm": return [f"ctx.t = ((ctx.r[0] & (uint32_t){args['imm']}) == 0);"]
    if mnem == "muls": return [f"ctx.macl = (uint32_t)((int32_t){R(n)} * (int32_t){R(m)});"]
    if mnem == "mulu": return [f"ctx.macl = {R(n)} * {R(m)};"]
    if mnem == "mull": return [f"ctx.macl = {R(n)} * {R(m)};"]
    if mnem == "not": return [f"{R(n)} = ~{R(m)};"]
    if mnem == "neg": return [f"{R(n)} = (uint32_t)(-(int32_t){R(m)});"]
    if mnem == "negc": return [f"{{ uint32_t _r = (uint32_t)(-(int32_t){R(m)}) - (ctx.t?1:0); ctx.t = ({R(m)} != 0) || ctx.t; {R(n)} = _r; }}"]
    if mnem == "swapb": return [f"{R(n)} = ({R(m)} & 0xFFFF0000u) | (({R(m)} & 0xFF) << 8) | (({R(m)} >> 8) & 0xFF);"]
    if mnem == "swapw": return [f"{R(n)} = ({R(m)} << 16) | ({R(m)} >> 16);"]
    if mnem == "extub": return [f"{R(n)} = {R(m)} & 0xFFu;"]
    if mnem == "extuw": return [f"{R(n)} = {R(m)} & 0xFFFFu;"]
    if mnem == "extsb": return [f"{R(n)} = (uint32_t)(int32_t)(int8_t){R(m)};"]
    if mnem == "extsw": return [f"{R(n)} = (uint32_t)(int32_t)(int16_t){R(m)};"]

    if mnem == "shll": return [f"ctx.t = ({R(n)} >> 31) & 1;", f"{R(n)} <<= 1;"]
    if mnem == "shlr": return [f"ctx.t = {R(n)} & 1;", f"{R(n)} >>= 1;"]
    if mnem == "shll2": return [f"{R(n)} <<= 2;"]
    if mnem == "shll8": return [f"{R(n)} <<= 8;"]
    if mnem == "shll16": return [f"{R(n)} <<= 16;"]
    if mnem == "shlr2": return [f"{R(n)} >>= 2;"]
    if mnem == "shlr8": return [f"{R(n)} >>= 8;"]
    if mnem == "shlr16": return [f"{R(n)} >>= 16;"]
    if mnem == "shal": return [f"ctx.t = ({R(n)} >> 31) & 1;", f"{R(n)} = (uint32_t)((int32_t){R(n)} << 1);"]
    if mnem == "shar": return [f"ctx.t = {R(n)} & 1;", f"{R(n)} = (uint32_t)((int32_t){R(n)} >> 1);"]

    if mnem == "lds_mem_pr":
        return [f"ctx.pr = MEM32({R(n)}); {R(n)} += 4;"]
    if mnem == "stsl_pr": return [f"{R(n)} -= 4; MEM32_WRITE({R(n)}, ctx.pr);"]
    if mnem == "stsl_mach": return [f"{R(n)} -= 4; MEM32_WRITE({R(n)}, ctx.mach);"]
    if mnem == "stsl_macl": return [f"{R(n)} -= 4; MEM32_WRITE({R(n)}, ctx.macl);"]
    if mnem == "stspr": return [f"{R(n)} = ctx.pr;"]
    if mnem == "movt": return [f"{R(n)} = ctx.t ? 1u : 0u;"]
    if mnem == "clrmac": return ["ctx.macl = 0; ctx.mach = 0;"]

    # SH7750 FPSCR.SZ selects 32-bit versus paired 64-bit FMOV transfers;
    # encoded odd F-registers select the XD bank in pair mode.  FPSCR.PR
    # selects single versus double arithmetic.  Keep these decisions in the
    # runtime helpers so every newly generated static function is exact in
    # both modes instead of requiring per-function repairs.
    if mnem == "fmov": return [f"host_fmov_reg(ctx, {n}, {m});"]
    if mnem == "fmovs_ld": return [f"host_fmov_load(ctx, {n}, {R(m)});"]
    if mnem == "fmovs_st": return [f"host_fmov_store(ctx, {R(n)}, {m});"]
    if mnem == "fmovs_ld_postinc": return [f"host_fmov_load_postinc(ctx, {n}, {m});"]
    if mnem == "fmovs_st_predec": return [f"host_fmov_store_predec(ctx, {n}, {m});"]
    if mnem == "fmovs_ld_r0": return [f"host_fmov_load(ctx, {n}, {R(m)} + ctx.r[0]);"]
    if mnem == "fmovs_st_r0": return [f"host_fmov_store(ctx, {R(n)} + ctx.r[0], {m});"]
    if mnem == "fadd": return [f"host_fadd(ctx, {n}, {m});"]
    if mnem == "fsub": return [f"host_fsub(ctx, {n}, {m});"]
    if mnem == "fmul": return [f"host_fmul(ctx, {n}, {m});"]
    if mnem == "fdiv": return [f"host_fdiv(ctx, {n}, {m});"]
    if mnem == "fcmp_eq": return [f"host_fcmp_eq(ctx, {n}, {m});"]
    if mnem == "fcmp_gt": return [f"host_fcmp_gt(ctx, {n}, {m});"]
    if mnem == "fldi0": return [f"ctx.frf[{n}] = 0.0f;"]
    if mnem == "fldi1": return [f"ctx.frf[{n}] = 1.0f;"]
    if mnem == "fneg": return [f"ctx.frf[{n}] = -ctx.frf[{n}];"]
    if mnem == "fabs_": return [f"ctx.frf[{n}] = fabsf(ctx.frf[{n}]);"]
    if mnem == "fsqrt": return [f"ctx.frf[{n}] = sqrtf(ctx.frf[{n}]);"]
    if mnem == "fsts_fpul": return [f"ctx.fr[{n}] = ctx.fpul;"]
    if mnem == "flds_fpul": return [f"ctx.fpul = ctx.fr[{n}];"]
    if mnem == "float_fpul": return [f"host_float_fpul(ctx, {n});"]
    if mnem == "ftrc_fpul": return [f"host_ftrc_fpul(ctx, {n});"]
    if mnem == "fneg_special": return [f"host_fneg(ctx, {n});"]
    if mnem == "fabs_special": return [f"host_fabs(ctx, {n});"]
    if mnem == "fsqrt_special": return [f"host_fsqrt(ctx, {n});"]
    if mnem == "fldi0_special": return [f"if (!ctx.fpscr_pr) ctx.frf[{n}] = 0.0f;"]
    if mnem == "fldi1_special": return [f"if (!ctx.fpscr_pr) ctx.frf[{n}] = 1.0f;"]
    if mnem == "fcnvsd": return [f"host_fcnvsd(ctx, {n});"]
    if mnem == "fcnvds": return [f"host_fcnvds(ctx, {n});"]
    if mnem == "fsca": return [f"host_fsca(ctx, {n});"]
    if mnem == "frchg": return ["host_frchg(ctx);"]

    if mnem == "sqrt" or mnem == "fsqrt": return [f"ctx.frf[{n}] = sqrtf(ctx.frf[{n}]);"]

    if mnem == "sts_mach": return [f"{R(n)} = ctx.mach;"]
    if mnem == "sts_macl": return [f"{R(n)} = ctx.macl;"]
    if mnem == "lds_mach": return [f"ctx.mach = {R(n)};"]
    if mnem == "lds_macl": return [f"ctx.macl = {R(n)};"]
    if mnem == "ldsl_mach": return [f"ctx.mach = MEM32({R(n)}); {R(n)} += 4;"]
    if mnem == "ldsl_macl": return [f"ctx.macl = MEM32({R(n)}); {R(n)} += 4;"]
    if mnem == "stcgbr": return [f"{R(n)} = ctx.gbr;"]
    if mnem == "ldcsr": return [f"load_sr(ctx, {R(n)});"]
    if mnem == "ldcgbr": return [f"ctx.gbr = {R(n)};"]
    if mnem == "ldcl_sr": return [f"load_sr(ctx, MEM32({R(n)})); {R(n)} += 4;"]
    if mnem == "dt": return [f"{R(n)} -= 1;", f"ctx.t = ({R(n)} == 0);"]
    if mnem == "cmp_pl": return [f"ctx.t = ((int32_t){R(n)} > 0);"]
    if mnem == "cmp_pz": return [f"ctx.t = ((int32_t){R(n)} >= 0);"]
    if mnem == "clrt": return ["ctx.t = false;"]
    if mnem == "sett": return ["ctx.t = true;"]
    if mnem == "div0u": return ["ctx.t = false; ctx.q = false; ctx.m_bit = false;"]
    if mnem == "div0s": return [f"ctx.q = ({R(n)} >> 31) & 1; ctx.m_bit = ({R(m)} >> 31) & 1; ctx.t = (ctx.q != ctx.m_bit);"]
    if mnem == "div1": return [f"host_div1(ctx, {n}, {m});  /* multi-step division primitive */"]
    if mnem == "shad": return [f"{{ int32_t _s = (int32_t){R(m)}; if (_s >= 0) {R(n)} <<= (_s & 31); else if ((_s & 31)==0) {R(n)} = ((int32_t){R(n)} < 0) ? 0xFFFFFFFFu : 0u; else {R(n)} = (uint32_t)((int32_t){R(n)} >> (-_s & 31)); }}"]
    if mnem == "shld": return [f"{{ int32_t _s = (int32_t){R(m)}; if (_s >= 0) {R(n)} <<= (_s & 31); else if ((_s & 31)==0) {R(n)} = 0; else {R(n)} >>= (-_s & 31); }}"]
    if mnem == "pref":
        n = args["n"]
        return [f"host_pref(ctx, {R(n)});"]

    if mnem == "fschg": return ["host_fschg(ctx);"]
    if mnem == "ftrc": return [f"ctx.fpul = (uint32_t)(int32_t)ctx.frf[{n}];"]
    if mnem == "float_": return [f"ctx.frf[{n}] = (float)(int32_t)ctx.fpul;"]
    if mnem == "fmac": return [f"ctx.frf[{n}] += ctx.frf[0] * ctx.frf[{m}];"]
    if mnem == "ftrv": return [f"host_ftrv(ctx, {n});  /* matrix-vector transform, 4x4 xmtrx * fvn */"]

    if mnem == "sts_fpscr": return [f"{R(n)} = ctx.fpscr;"]
    if mnem == "sts_fpul": return [f"{R(n)} = ctx.fpul;"]
    if mnem == "lds_fpscr": return [f"host_load_fpscr(ctx, {R(n)});"]
    if mnem == "lds_fpul": return [f"ctx.fpul = {R(n)};"]
    if mnem == "lds_mem_fpscr": return [f"host_load_fpscr(ctx, MEM32({R(n)})); {R(n)} += 4;"]
    if mnem == "lds_mem_fpul": return [f"ctx.fpul = MEM32({R(n)}); {R(n)} += 4;"]
    if mnem == "stsl_fpscr": return [f"{R(n)} -= 4; MEM32_WRITE({R(n)}, ctx.fpscr);"]
    if mnem == "dmulu": return [f"{{ uint64_t _p = (uint64_t){R(n)} * {R(m)}; ctx.macl = (uint32_t)_p; ctx.mach = (uint32_t)(_p >> 32); }}"]
    if mnem == "rotcl": return [f"{{ bool _t = ({R(n)} >> 31) & 1; {R(n)} = ({R(n)} << 1) | (ctx.t ? 1u : 0u); ctx.t = _t; }}"]
    if mnem == "rotcr": return [f"{{ bool _t = {R(n)} & 1; {R(n)} = ({R(n)} >> 1) | (ctx.t ? 0x80000000u : 0u); ctx.t = _t; }}"]
    if mnem == "rotl": return [f"ctx.t = ({R(n)} >> 31) & 1; {R(n)} = ({R(n)} << 1) | ({R(n)} >> 31);"]
    if mnem == "rotr": return [f"ctx.t = {R(n)} & 1; {R(n)} = ({R(n)} >> 1) | ({R(n)} << 31);"]
    if mnem == "fipr": return [f"host_fipr(ctx, {m}, {n});  /* FIPR FVm,FVn */"]
    if mnem == "fsrra": return [f"if (!ctx.fpscr_pr) ctx.frf[{n}] = 1.0f / sqrtf(ctx.frf[{n}]);"]

    if mnem == "stc_sr":
        return [f"{R(n)} = (ctx.t ? 1u : 0u) | (ctx.q ? 0x100u : 0u) | (ctx.m_bit ? 0x200u : 0u);"]
    if mnem == "stc_vbr":
        return [f"{R(n)} = ctx.vbr;"]
    if mnem in ("ocbi", "ocbp", "ocbwb"):
        return [f"/* {mnem} @Rn -- operand cache block hint, no-op on host */"]
    if mnem == "xtrct":
        return [f"{R(n)} = ({R(n)} >> 16) | ({R(m)} << 16);"]

    if mnem == "rts": return ["return;  // rts"]
    return [f"/* UNHANDLED: {mnem} {args} at 0x{addr:08X} */"]


def gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end):
    """Generate C++ for a delay-slot instruction, correctly handling the
    case where the delay-slot instruction is itself an unconditional branch
    or call -- a valid SH-4 pattern gen_simple() alone can't handle, since
    branch targets need function-range context to decide goto-vs-call."""
    if d_mnem == "bra":
        target = d_addr + d_args["disp"]
        if func_start <= target < func_end:
            return [f"goto L_{target:08X};"]
        return [f"FUN_{target:08X}(ctx); return;  /* tail-jump outside function, from delay slot */"]
    if d_mnem == "bsr":
        target = d_addr + d_args["disp"]
        return [f"ctx.pr = 0x{d_addr+4:08X}u; FUN_{target:08X}(ctx); return;  /* call from delay slot */"]
    if d_mnem == "jmp":
        reg = d_args["n"]
        return [f"host_indirect_call(ctx, ctx.r[{reg}]); return;  /* indirect jump from delay slot */"]
    if d_mnem == "jsr":
        reg = d_args["n"]
        return [f"ctx.pr = 0x{d_addr+4:08X}u; host_indirect_call(ctx, ctx.r[{reg}]); return;  /* indirect call from delay slot */"]
    if d_mnem in ("bt", "bf"):
        # A conditional branch in another branch's delay slot: both read the
        # SAME, still-current T-flag (nothing between them changes it), so
        # this is a valid compiler pattern -- effectively packing a full
        # if/else into one delay-slot pair, since T can only be 0 or 1.
        target = d_addr + d_args["disp"]
        check = "ctx.t" if d_mnem == "bt" else "!ctx.t"
        if func_start <= target < func_end:
            return [f"if ({check}) goto L_{target:08X};"]
        return [f"if ({check}) {{ FUN_{target:08X}(ctx); return; }}  /* tail-branch outside function, from delay slot */"]
    return gen_simple(d_mnem, d_args, d_addr)


def recompile_function(name, base_addr, raw_bytes, unresolved_ctr=None, resolved_calls=None, empirical_calls=None, trace=False, internal_braf_targets=None, reachable_addrs=None):
    resolved_calls = resolved_calls or {}
    empirical_calls = empirical_calls or {}
    n_insns = len(raw_bytes) // 2
    ops = []
    for i in range(n_insns):
        addr = base_addr + i * 2
        word = raw_bytes[i*2] | (raw_bytes[i*2+1] << 8)
        mnem, args = decode(word)
        ops.append((addr, mnem, args))
        if mnem == "unk" and unresolved_ctr is not None and (reachable_addrs is None or addr in reachable_addrs):
            unresolved_ctr[args["raw"]] = unresolved_ctr.get(args["raw"], 0) + 1

    func_start = base_addr
    func_end = base_addr + len(raw_bytes)

    internal_braf_targets = internal_braf_targets or {}
    labels = set()
    for _site, _targets in internal_braf_targets.items():
        for _t in _targets:
            if func_start <= _t < func_end:
                labels.add(_t)
    for addr, mnem, args in ops:
        if reachable_addrs is not None and addr not in reachable_addrs:
            continue
        if mnem in ("bt", "bf", "bt_s", "bf_s", "bra", "bsr"):
            target = addr + args["disp"]
            if func_start <= target < func_end:
                labels.add(target)

    out = [f"void {name}(SH4Context& ctx) {{"]
    if trace:
        out.append(f"trace_call(0x{base_addr:08X}u);")
    i = 0
    while i < len(ops):
        addr, mnem, args = ops[i]
        if reachable_addrs is not None and addr not in reachable_addrs:
            i += 1
            continue
        if addr in labels:
            out.append(f"L_{addr:08X}:")

        # SH-4 RTS has a mandatory delay slot: the instruction at PC+2
        # executes BEFORE control returns to PR.  Older generated snapshots
        # emitted `return` first, silently skipping this final instruction.
        if mnem == "rts":
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]
                if d_addr in labels:
                    out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            out.append("return;  // rts (after delay slot)")
            i += 2
            continue

        # SH-4: BT and BF have NO delay slot -- only BT/S and BF/S do. Treating
        # bt/bf as delayed makes the following instruction execute even when the
        # branch is taken, which corrupts control flow (e.g. the free-list walk
        # in 0x0C0204C0 span into an infinite loop).
        if mnem in ("bt", "bf"):
            target = addr + args["disp"]
            check = "ctx.t" if mnem == "bt" else "!ctx.t"
            if func_start <= target < func_end:
                out.append(f"if ({check}) goto L_{target:08X};")
            else:
                out.append(f"if ({check}) {{ FUN_{target:08X}(ctx); return; }}  /* tail-branch outside function */")
            i += 1
            continue

        if mnem in ("bt_s", "bf_s"):
            target = addr + args["disp"]
            takes_if_true = mnem in ("bt", "bt_s")
            d_addr, d_mnem, d_args = (None, None, None)
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]

            if d_addr is not None and d_addr in labels:
                # The delay-slot instruction is also jumped to directly from
                # elsewhere. Use the live flag (correct for a direct-jump
                # entry) instead of a captured local, which also sidesteps
                # the goto/scoping restriction on local variable declarations.
                out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
                check = "ctx.t" if takes_if_true else "!ctx.t"
                if func_start <= target < func_end:
                    out.append(f"if ({check}) goto L_{target:08X};")
                else:
                    out.append(f"if ({check}) {{ FUN_{target:08X}(ctx); return; }}  /* tail-branch outside function */")
                i += 2
                continue

            cond_var = f"branch_cond_{addr:08X}"
            is_branch_delay = d_mnem in ("bra", "bsr", "jmp", "jsr")

            if is_branch_delay:
                # A branch in the delay slot of another branch: real SH-4
                # semantics here are that the outer conditional, if taken,
                # overrides the delay slot's own redirect entirely -- only
                # if the outer branch is NOT taken does the delay-slot
                # branch's own effect apply. This is the opposite order
                # from a normal (non-branching) delay-slot instruction.
                out.append("{")
                out.append(f"bool {cond_var} = ctx.t;")
                check = cond_var if takes_if_true else f"!{cond_var}"
                if func_start <= target < func_end:
                    out.append(f"if ({check}) goto L_{target:08X};")
                else:
                    out.append(f"if ({check}) {{ FUN_{target:08X}(ctx); return; }}  /* tail-branch outside function */")
                out.append("}")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
                i += 2
                continue

            out.append("{")  # scope the condition var so it can't cross a later label
            out.append(f"bool {cond_var} = ctx.t;")
            if d_addr is not None:
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            check = cond_var if takes_if_true else f"!{cond_var}"
            if func_start <= target < func_end:
                out.append(f"if ({check}) goto L_{target:08X};")
            else:
                out.append(f"if ({check}) {{ FUN_{target:08X}(ctx); return; }}  /* tail-branch outside function */")
            out.append("}")
            i += 2
            continue

        if mnem in ("bra", "bsr"):
            target = addr + args["disp"]
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]
                if d_addr in labels:
                    out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            if mnem == "bra":
                if func_start <= target < func_end:
                    out.append(f"goto L_{target:08X};")
                else:
                    out.append(f"FUN_{target:08X}(ctx); return;  /* tail-jump outside function */")
            else:
                out.append(f"ctx.pr = 0x{addr+4:08X}u; FUN_{target:08X}(ctx);")
            i += 2
            continue

        if mnem == "jmp":
            reg = args["n"]
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]
                if d_addr in labels:
                    out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            out.append(f"host_indirect_call(ctx, ctx.r[{reg}]); return;  /* indirect jump */")
            i += 2
            continue

        if mnem == "jsr":
            reg = args["n"]
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]
                if d_addr in labels:
                    out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            addr_key = f"{addr:08x}"
            target_hex = resolved_calls.get(addr_key)
            # This call site joins two guest branches that install different
            # callbacks. A flat reference map can record only one target and
            # would silently replace the other callback. Keep the runtime R1
            # selection; host_indirect_call still dispatches only to static
            # AOT-translated functions.
            if addr_key == "0c1f10c8":
                target_hex = None
            emp_target = empirical_calls.get(addr_key)
            HOST_SHIMS = {
                "0C04C860": "shim_file_exists",
                "0C04E480": "shim_load2app_mem",
                "0C20C740": "shim_maple_jvs_poll",
                "0C020160": "shim_byte_fill",
                "0C2223E0": "shim_udiv",
                "0C0DDC60": "shim_code_lookup",
                "0C1EBEC0": "shim_pvr_status_wait",
                "0C2108C0": "shim_pvr_command_init",
                "0C203F40": "shim_software_delay",
                "0C1FAAE0": "shim_debug_text",
            }
            if target_hex and 0x0F000000 <= int(target_hex, 16) < 0x0F001C00 and (int(target_hex, 16) & 3) == 0:
                out.append(f"ctx.pr = 0x{addr+4:08X}u; shim_bios_hle(ctx, 0x{int(target_hex,16):08X}u);  "
                            f"/* host-bridged: real proven BIOS service HLE */")
            elif target_hex and target_hex.upper() in HOST_SHIMS:
                out.append(f"ctx.pr = 0x{addr+4:08X}u; {HOST_SHIMS[target_hex.upper()]}(ctx);  "
                            f"/* host-bridged: real hardware-talking code replaced with host I/O */")
            elif target_hex:
                out.append(f"ctx.pr = 0x{addr+4:08X}u; FUN_{target_hex.upper()}(ctx);  /* resolved via static analysis */")
            elif emp_target is not None:
                out.append(f"ctx.pr = 0x{addr+4:08X}u;")
                out.append(f"if (ctx.r[{reg}] == 0x{emp_target:08X}u) {{ FUN_{emp_target:08X}(ctx); }}"
                            f" else {{ host_indirect_call(ctx, ctx.r[{reg}]); }}"
                            f"  /* guarded: 0x{emp_target:08X} observed in real gameplay capture, verified at runtime */")
            else:
                out.append(f"ctx.pr = 0x{addr+4:08X}u;")
                out.append(f"host_indirect_call(ctx, ctx.r[{reg}]);  /* genuinely dynamic target */")
            i += 2
            continue

        if mnem in ("braf", "bsrf"):
            reg = args["n"]
            target_var = f"computed_branch_target_{addr:08X}"
            out.append("{")
            out.append(f"const uint32_t {target_var} = 0x{addr+4:08X}u + ctx.r[{reg}];")
            if i + 1 < len(ops):
                d_addr, d_mnem, d_args = ops[i+1]
                if d_addr in labels:
                    out.append(f"L_{d_addr:08X}:")
                out.extend(gen_delay_slot(d_addr, d_mnem, d_args, func_start, func_end))
            target = target_var
            if mnem == "bsrf":
                out.append(f"ctx.pr = 0x{addr+4:08X}u;")
                out.append(f"host_indirect_call(ctx, {target});  /* BSRF: linking computed branch */")
            else:
                _targets = internal_braf_targets.get(addr, [])
                if _targets:
                    out.append(f"switch ({target}) {{")
                    for _t in _targets:
                        if func_start <= _t < func_end:
                            out.append(f"case 0x{_t:08X}u: goto L_{_t:08X};")
                        else:
                            out.append(f"case 0x{_t:08X}u: host_indirect_call(ctx, 0x{_t:08X}u); return;")
                    out.append(f"default: host_indirect_call(ctx, {target}); return;")
                    out.append("}")
                else:
                    out.append(f"host_indirect_call(ctx, {target});  /* BRAF: non-linking computed branch */")
                    out.append("return;  /* BRAF does not execute fallthrough */")
            out.append("}")
            i += 2
            continue

        out.extend(gen_simple(mnem, args, addr))
        i += 1

    if out and out[-1].endswith(":") and not out[-1].startswith("//"):
        out.append(";")  # a label can't be immediately followed by a closing brace pre-C++23
    out.append("}")
    return "\n".join(out)
