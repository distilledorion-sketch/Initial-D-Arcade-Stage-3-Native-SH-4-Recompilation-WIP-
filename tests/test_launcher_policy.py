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
INTEGRITY = (
    ROOT / "tools" / "windows" / "GameFilesIntegrity.ps1"
).read_text(encoding="utf-8")


class LauncherPolicyTests(unittest.TestCase):
    def test_presentation_mode_is_a_validated_launcher_parameter(self):
        self.assertIn("[ValidateSet('Safe', 'Direct')]", LAUNCHER)
        self.assertIn("[string]$VulkanPresentation = 'Direct'", LAUNCHER)

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

    def test_public_default_uses_direct_vulkan(self):
        self.assertIn("VulkanPresentation=Direct", PUBLIC_SETTINGS)

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

    def test_extracted_files_get_bounded_retry_before_chd_setup(self):
        integrity = LAUNCHER.split("$gameFilesReady = Test-Idas3GameFiles", 1)[1]
        integrity = integrity.split("if (-not $ValidateOnly", 1)[0]
        self.assertIn("Start-Sleep -Milliseconds 200", integrity)
        self.assertEqual(
            LAUNCHER.count(
                "Test-Idas3GameFiles -Root $gameFilesRoot -Quiet"
            ),
            2,
        )
        self.assertNotIn("-FullHash", integrity)

    def test_game_file_integrity_checker_is_a_packaged_source(self):
        self.assertIn("function Test-Idas3GameFiles", INTEGRITY)
        self.assertIn("@($manifest.files).Count -ne 2196", INTEGRITY)
        self.assertIn("$main.Length -ne 4194304", INTEGRITY)
        self.assertIn(
            "EFDA831F1212DB54CC2E4BA53424FE390F91B2DAABB07E93C1E389C3736D0335",
            INTEGRITY,
        )
        self.assertIn("$relative.Contains('..')", INTEGRITY)
        self.assertIn("$candidate.StartsWith(", INTEGRITY)

    def test_normal_integrity_pass_hashes_assets_only_when_metadata_changed(self):
        self.assertIn(
            "if ($FullHash -or $mtimeNs -ne [int64]$record.mtime_ns)",
            INTEGRITY,
        )
        asset_hash = INTEGRITY.index(
            "Get-FileHash -Algorithm SHA256 -LiteralPath $candidate"
        )
        metadata_gate = INTEGRITY.index(
            "if ($FullHash -or $mtimeNs -ne [int64]$record.mtime_ns)"
        )
        self.assertLess(metadata_gate, asset_hash)

    def test_asset_walk_avoids_two_powershell_provider_calls_per_file(self):
        asset_walk = INTEGRITY.split("$hostfsPrefix =", 1)[1].split(
            "return $true", 1
        )[0]
        self.assertIn("[System.IO.File]::Exists($candidate)", asset_walk)
        self.assertIn("[System.IO.FileInfo]::new($candidate)", asset_walk)
        self.assertNotIn("Test-Path -LiteralPath $candidate", asset_walk)
        self.assertNotIn("Get-Item -LiteralPath $candidate", asset_walk)

    def test_saved_difficulty_is_validated_and_forwarded(self):
        self.assertIn("$gameDifficulty = 'Normal'", LAUNCHER)
        self.assertIn("$saved.Difficulty -in @(\n", LAUNCHER)
        for level in ("VeryEasy", "Easy", "Normal", "Hard", "VeryHard"):
            self.assertIn("'%s'" % level, LAUNCHER)
        self.assertIn("$env:IDAS3_GAME_DIFFICULTY = $gameDifficulty", LAUNCHER)
        self.assertIn("Difficulty = $gameDifficulty", LAUNCHER)
        self.assertIn("Difficulty=Normal", PUBLIC_SETTINGS)

    def test_launcher_checks_for_updates_before_game_setup(self):
        update_call = LAUNCHER.index("Invoke-Idas3UpdateCheck")
        setup_call = LAUNCHER.index("function Find-GameInputFile")
        self.assertLess(update_call, setup_call)
        self.assertIn("$ProductVersion = 2489", LAUNCHER)
        self.assertIn("[switch]$SkipUpdateCheck", LAUNCHER)

    def test_one_launcher_owns_each_installation_before_expensive_work(self):
        acquire = LAUNCHER.index("$script:launcherMutex.WaitOne(0)")
        update = LAUNCHER.index("Invoke-Idas3UpdateCheck")
        integrity = LAUNCHER.index(
            "$gameFilesReady = Test-Idas3GameFiles"
        )
        self.assertLess(acquire, update)
        self.assertLess(acquire, integrity)
        self.assertIn("Local\\IDAS3RecompLauncher_$mutexId", LAUNCHER)
        self.assertIn("System.Threading.AbandonedMutexException", LAUNCHER)
        self.assertIn(
            "already checking files or preparing an update", LAUNCHER
        )

    def test_existing_runtime_is_rejected_before_update_or_integrity_work(self):
        process_check = LAUNCHER.index("$runtimeProcessNames = @(")
        update = LAUNCHER.index("Invoke-Idas3UpdateCheck")
        integrity = LAUNCHER.index(
            "$gameFilesReady = Test-Idas3GameFiles"
        )
        self.assertLess(process_check, update)
        self.assertLess(process_check, integrity)
        self.assertIn("$canonicalExecutable", LAUNCHER[process_check:update])
        self.assertIn("$legacyExecutable", LAUNCHER[process_check:update])
        self.assertIn("Get-Process -Name $_", LAUNCHER[process_check:update])

    def test_launcher_mutex_is_released_on_every_explicit_exit(self):
        self.assertIn("function Close-Idas3LauncherMutex", LAUNCHER)
        self.assertIn("$script:launcherMutex.ReleaseMutex()", LAUNCHER)
        self.assertIn("$script:launcherMutex.Dispose()", LAUNCHER)
        self.assertGreaterEqual(
            LAUNCHER.count("Close-Idas3LauncherMutex"), 5
        )

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

    def test_update_prompt_is_visible_closeable_and_time_bounded(self):
        self.assertIn("$form.ShowInTaskbar = $true", UPDATER)
        self.assertIn("$form.Add_FormClosing", UPDATER)
        self.assertIn("$form.Tag = 'no'", UPDATER)
        self.assertIn("$promptTimeout.Interval = 30000", UPDATER)
        self.assertIn("$promptTimeout.Start()", UPDATER)
        self.assertIn("$promptTimeout.Dispose()", UPDATER)

    def test_updater_verifies_github_digest_and_package_version(self):
        self.assertIn("sha256:", UPDATER.lower())
        self.assertIn("Get-FileHash -LiteralPath $ZipPath -Algorithm SHA256", UPDATER)
        self.assertIn("PRODUCT_VERSION.txt", UPDATER)
        self.assertIn("Unsafe path in update package", UPDATER)

    def test_failed_staging_cleanup_is_confined_to_updater_local_data(self):
        self.assertIn("function Remove-Idas3UpdateWorkingDirectory", UPDATER)
        cleanup = UPDATER.split(
            "function Remove-Idas3UpdateWorkingDirectory", 1
        )[1].split("function Get-Idas3AutomaticUpdatePreference", 1)[0]
        self.assertIn("InitialDAS3Recomp\\updates", cleanup)
        self.assertIn("[System.IO.Path]::GetFullPath", cleanup)
        self.assertIn("$resolved.StartsWith(", cleanup)
        self.assertIn("[System.IO.Directory]::Delete($resolved, $true)", cleanup)
        staging = UPDATER.split("function Start-Idas3StagedUpdate", 1)[1]
        self.assertIn(
            "Remove-Idas3UpdateWorkingDirectory -WorkingRoot $workingRoot",
            staging,
        )

    def test_installer_never_writes_after_parent_wait_timeout(self):
        wait = INSTALLER.index("$process.WaitForExit(60000)")
        marker = INSTALLER.index("$markerPath = Join-Path $source")
        first_copy = INSTALLER.index("Copy-Item -LiteralPath $file.FullName")
        self.assertLess(wait, marker)
        self.assertLess(wait, first_copy)
        self.assertIn(
            "Timed out waiting for launcher process $waitId to close",
            INSTALLER,
        )

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
