# WPF Inspection App Handoff - User Light Expression UI

Date: 2026-05-20  
Project: `C:\Users\USER\Documents\NewProject\ImageProject`  
Reference project: `D:\Work\GGY\pemtoFrameworkAll_R_4.0.0.7`

## 1. Principles

- Do not copy the full reference UI. Adopt only the useful User Light behavior.
- Keep XAML as presentation and binding only.
- Keep User Light composition, preview branching, operator cycling, and preset logic in ViewModel/Service code.
- Do not add code-behind event handlers.

## 2. Reference Files

- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_User.cs`
- `pemtoFramework\01.UI\UserControl\Teaching\LightColor\ucLightControl_User.Designer.cs`

Observed behavior adopted:

- User Light cells are shown as a visual expression with TOP/MIDDLE/BOTTOM position and RGBW summary.
- Operator buttons cycle through `None -> Add -> Sub -> None`.
- Setting the last blank operator to `+` or `-` appends the next cell automatically.
- Returning the second-last operator to blank removes the trailing cell automatically.
- `MIX` previews the whole User Light expression.
- `CURRENT` previews only the selected cell.
- A channel preset sets the selected channel to 100 and clears the other editable channels.

## 3. Work Completed

### 3.1 User Light Expression Cards

- Replaced the previous small chip list with expression-style User Light cards.
- Each card shows:
  - display index
  - TOP/MID/BTM position
  - three-band position indicator
  - current RGBW summary
  - selected state
- Added an operator button next to each card so the composition reads like the reference project.

### 3.2 Operator Flow

- Added `ToggleUserCellOperatorCommand`.
- Middle cells switch between `+` and `-`.
- Last and second-last cells cycle through blank, `+`, `-`, blank.
- Appends a new trailing cell when the last blank operator becomes `+` or `-`.
- Removes the trailing cell when the second-last operator returns to blank.

### 3.3 MIX / CURRENT Preview

- Added a `MIX` / `CURRENT` preview toggle for User Light.
- Stored data still keeps the complete User Light expression.
- `CreatePreviewState()` creates a one-cell state only for selected-cell preview.

### 3.4 Channel Preset Relocation

- Removed the separate R/G/B/W preset row.
- Made the left channel-name area in the selected-cell slider rows act as the preset button.
- Clicking `R`, `G`, `B`, or `W` sets that channel to 100 and clears the other editable channels.
- The command is now local to `LightChannelViewModel.PresetCommand`, so it no longer depends on a fragile ancestor binding.

## 4. Changed Files

- `src/WpfInspectionPrototype/WpfInspectionApp/Views/LightControlView.xaml`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/LightControlViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/ViewModels/MainViewModel.cs`
- `src/WpfInspectionPrototype/WpfInspectionApp/Diagnostics/SmokeTestRunner.cs`
- `docs/2026-05-20-handoff-user-light-expression-ui.md`

## 5. MVVM Check

- `LightControlView.xaml.cs` was not changed.
- `LightControlView.xaml` has no direct `Click=`, `SelectionChanged=`, `ValueChanged=`, `TextChanged=`, `MouseUp=`, or `PreviewMouse` handlers.
- Light raw parameters did not move into View or code-behind.
- User Light composition, operator switching, channel presets, and preview state branching are handled by ViewModel commands/properties.

## 6. Verification

Build:

```powershell
cd C:\Users\USER\Documents\NewProject\ImageProject\src\WpfInspectionPrototype
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' .\WpfInspectionPrototype.sln /m /p:Configuration=Debug /p:Platform=x64 /v:minimal
```

Result: success

Smoke:

```powershell
cd C:\Users\USER\Documents\NewProject\ImageProject\src\WpfInspectionPrototype\WpfInspectionApp\bin\x64\Debug\net48
.\WpfInspectionApp.exe --smoke-test --software-rendering
```

Result: success

Additional checks:

- Smoke test directly executes the channel-name preset command and verifies the expected `B100/R0` behavior.
- Four MVVM/event-handler `rg` checks returned no matches.

## 7. Remaining Manual Checks

- In the actual GUI, select a User Light cell and click the left `R/G/B/W` channel names.
- Confirm that `CURRENT` preview feels like selected-cell-only preview with real PTT data.
- If the reference project's right-click additive preset behavior is needed later, add it as a separate MVVM command.
