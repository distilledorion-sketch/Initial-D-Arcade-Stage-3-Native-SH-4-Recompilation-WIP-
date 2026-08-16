"""Regression tests for SH7750 FPSCR-aware static FPU code generation.

The opcodes and width/bank rules are from the Hitachi SH7750 Programming
Manual, sections 6.3.2, 6.6.2, and 10.37-10.42.  These tests intentionally
exercise encodings that previously needed hand repairs in generated game
functions.
"""

from pathlib import Path
import sys
import unittest


TRANSLATOR = Path(__file__).resolve().parents[1] / "translator"
sys.path.insert(0, str(TRANSLATOR))

import codegen  # noqa: E402
import sh4recomp  # noqa: E402


class Sh4FpuCodegenTests(unittest.TestCase):
    def generated(self, opcode: int) -> list[str]:
        mnemonic, args = sh4recomp.decode(opcode)
        return codegen.gen_simple(mnemonic, args, 0x0C000000)

    def test_fmov_register_uses_sz_aware_helper(self) -> None:
        # F10C: with SZ=1 this is XD0 <- DR0, not scalar FR1 <- FR0.
        self.assertEqual(self.generated(0xF10C), ["host_fmov_reg(ctx, 1, 0);"])

    def test_fmov_postincrement_uses_pair_width(self) -> None:
        # F179: SZ=0 advances R7 by 4; SZ=1 loads XD0 and advances by 8.
        self.assertEqual(
            self.generated(0xF179),
            ["host_fmov_load_postinc(ctx, 1, 7);"],
        )

    def test_fmov_predecrement_keeps_gpr_and_freg_operand_order(self) -> None:
        # F4FB: address is R4 and encoded floating source is 15 (XD7 in SZ=1).
        self.assertEqual(
            self.generated(0xF4FB),
            ["host_fmov_store_predec(ctx, 4, 15);"],
        )

    def test_fmov_indexed_forms_use_sz_aware_helpers(self) -> None:
        self.assertEqual(
            self.generated(0xF126),
            ["host_fmov_load(ctx, 1, ctx.r[2] + ctx.r[0]);"],
        )
        self.assertEqual(
            self.generated(0xF127),
            ["host_fmov_store(ctx, ctx.r[1] + ctx.r[0], 2);"],
        )

    def test_arithmetic_and_comparisons_use_pr_aware_helpers(self) -> None:
        expected = {
            0xF020: "host_fadd(ctx, 0, 2);",
            0xF021: "host_fsub(ctx, 0, 2);",
            0xF022: "host_fmul(ctx, 0, 2);",
            0xF023: "host_fdiv(ctx, 0, 2);",
            0xF024: "host_fcmp_eq(ctx, 0, 2);",
            0xF025: "host_fcmp_gt(ctx, 0, 2);",
        }
        for opcode, line in expected.items():
            with self.subTest(opcode=f"{opcode:04X}"):
                self.assertEqual(self.generated(opcode), [line])


if __name__ == "__main__":
    unittest.main()
