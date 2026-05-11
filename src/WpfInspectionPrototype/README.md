# WPF Inspection Prototype

This prototype replaces the WebView2/Base64 image path with a direct WPF + native-buffer path.

## Projects

- `WpfInspectionApp`: WPF UI, model save/load, image viewer, threshold controls, ROI drawing, result display.
- `BridgeLayer`: C++ bridge DLL exported through P/Invoke. This layer owns C# to C++ boundary checks and exception handling.
- `InspectionCore`: C++ inspection DLL placeholder. Current implementation provides BGRA to Gray8 threshold and result bounds.

## Data Flow

1. WPF loads `Image/2D.jpg` once and converts it to a BGRA byte buffer.
2. Threshold changes are debounced in WPF.
3. C# pins the source and destination arrays.
4. `BridgeLayer.dll` receives raw pointers through P/Invoke.
5. `InspectionCore.dll` writes the binary Gray8 result buffer.
6. WPF updates a `WriteableBitmap` without JSON, Base64, WebView2 messages, or canvas readback.

## Build

Open `WpfInspectionPrototype.sln` in Visual Studio 2022 and build `Debug|x64`.

Command line:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe' .\WpfInspectionPrototype.sln /m /p:Configuration=Debug /p:Platform=x64
```

## Next Native Slots

- Replace `CoreThresholdBgraToGray8` with OpenCV `cv::Mat` input wrappers.
- Add bridge APIs for Blob, Scratch, OCR preprocessing, and result arrays.
- Return structured result buffers instead of strings for production inspection results.
