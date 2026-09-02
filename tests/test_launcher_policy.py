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
UPDATER = (ROOT / "tools" / "windows" / "UpdateSupport.ps1").read_text(
    encoding="utf-8"
)
INSTALLER = (ROOT / "tools" / "windows" / "Install Update.ps1").read_text(
    encoding="utf-8"
)


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

    def test_launcher_checks_for_updates_before_game_setup(self):
        update_call = LAUNCHER.index("Invoke-Idas3UpdateCheck")
        setup_call = LAUNCHER.index("function Find-GameInputFile")
        self.assertLess(update_call, setup_call)
        self.assertIn("$ProductVersion = 2459", LAUNCHER)
        self.assertIn("[switch]$SkipUpdateCheck", LAUNCHER)

    def test_canonical_main_and_runtime_names_are_migration_safe(self):
        self.assertIn("Initial D Arcade Stage 3 Recompiled Runtime.exe", LAUNCHER)
        self.assertIn("$legacyExecutable = Join-Path $buildRoot 'demo.exe'", LAUNCHER)
        self.assertIn("$PSBoundParameters.ContainsKey('Exe')", LAUNCHER)
        self.assertIn("if ($canonicalHash -eq $legacyHash)", LAUNCHER)
        self.assertIn("Initial D Arcade Stage 3 Recompiled.exe", UPDATER)
        self.assertIn("Initial D Arcade Stage 3 Recompiled Runtime.exe", UPDATER)
        self.assertIn("Initial D Arcade Stage 3 Recompiled.exe", INSTALLER)
        self.assertIn("Initial D Arcade Stage 3 Recompiled Runtime.exe", INSTALLER)

    def test_update_prompt_includes_version_date_and_automatic_choice(self):
        self.assertIn('"Update $($Update.VersionLabel) is available"', UPDATER)
        self.assertIn('"Released $($Update.ReleaseDateText)', UPDATER)
        self.assertIn("Automatically install future updates", UPDATER)
        self.assertIn("User deferred $($update.VersionLabel)", UPDATER)

    def test_updater_verifies_github_digest_and_package_version(self):
        self.assertIn("sha256:", UPDATER.lower())
        self.assertIn("Get-FileHash -LiteralPath $ZipPath -Algorithm SHA256", UPDATER)
        self.assertIn("PRODUCT_VERSION.txt", UPDATER)
        self.assertIn("Unsafe path in update package", UPDATER)

    def test_installer_preserves_user_owned_data_and_has_rollback(self):
        for directory in ("game files", "card data", "custom music", "logs"):
            self.assertIn(directory, INSTALLER)
        self.assertIn("idas3_user_settings.ini", INSTALLER)
        self.assertIn("idas3_launcher_update_settings.json", INSTALLER)
        self.assertIn("$rollback", INSTALLER)
        self.assertIn("Merge updates never delete unknown destination files", INSTALLER)
        self.assertIn("if ($canonicalHash -eq $legacyHash)", INSTALLER)

    def test_update_sources_contain_no_private_absolute_path(self):
        for source in (UPDATER, INSTALLER):
            self.assertNotIn("C:\\Users\\", source)
            self.assertNotIn("Claude_Handoffs", source)


if __name__ == "__main__":
    unittest.main()
