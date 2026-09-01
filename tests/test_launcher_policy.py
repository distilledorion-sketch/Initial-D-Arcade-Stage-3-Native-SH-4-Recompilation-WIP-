#!/usr/bin/env python3
"""Static contracts for the source-safe Windows public-demo launcher."""

from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]
LAUNCHER = (ROOT / "tools" / "windows" / "Play Demo.ps1").read_text(
    encoding="utf-8"
)
PUBLIC_SETTINGS = (
    ROOT / "tools" / "windows" / "idas3_user_settings_public.ini"
).read_text(encoding="utf-8")


class LauncherPolicyTests(unittest.TestCase):
    def test_presentation_mode_is_a_validated_launcher_parameter(self):
        self.assertIn("[ValidateSet('Safe', 'Direct')]", LAUNCHER)
        self.assertIn("[string]$VulkanPresentation = 'Safe'", LAUNCHER)

    def test_saved_f1_choice_is_loaded_unless_command_line_overrides_it(self):
        self.assertIn(
            "$PSBoundParameters.ContainsKey('VulkanPresentation')", LAUNCHER
        )
        self.assertIn("$saved.VulkanPresentation", LAUNCHER)

    def test_direct_and_safe_share_one_runtime_mapping(self):
        helper = LAUNCHER.split("function Get-VulkanOffscreenFlag", 1)[1]
        self.assertIn("if ($Mode -eq 'Direct') { return '0' }", helper)
        self.assertIn("return '1'", helper)
        self.assertGreaterEqual(
            LAUNCHER.count("Get-VulkanOffscreenFlag $VulkanPresentation"), 2
        )
        self.assertIn("$env:IDAS3_NATIVE_VULKAN_OFFSCREEN", LAUNCHER)

    def test_public_default_remains_safe(self):
        self.assertIn("VulkanPresentation=Safe", PUBLIC_SETTINGS)

    def test_launcher_contains_no_private_absolute_path(self):
        self.assertNotIn("C:\\Users\\", LAUNCHER)
        self.assertNotIn("Claude_Handoffs", LAUNCHER)

    def test_moved_card_diagnostic_does_not_contaminate_card_path(self):
        self.assertIn(
            'Write-Verbose "Repaired moved card setting: $portableCardPath"',
            LAUNCHER,
        )
        self.assertNotIn(
            'Write-Output "Repaired moved card setting: $portableCardPath"',
            LAUNCHER,
        )

    def test_extracted_files_get_content_retry_before_chd_setup(self):
        integrity = LAUNCHER.split("$gameFilesReady = Test-Idas3GameFiles", 1)[1]
        integrity = integrity.split("if (-not $ValidateOnly", 1)[0]
        self.assertIn("-FullHash -Quiet", integrity)
        self.assertIn("Start-Sleep -Milliseconds 200", integrity)


if __name__ == "__main__":
    unittest.main()
