# flutter_silk

A flutter ffi plugin for converting audio from silk to pcm/mp3. Support Macos, Ios, Linux, Windows and Android.

## Use Case

```dart
final silkData = File("input.silk").readAsBytesSync();
var output = silkToMp3(silkData);
File("output.mp3").writeAsBytesSync(output!);
```

For a complete example, please go to `/example` folder.

## Project structure

This template uses the following structure:

* `src`: Contains the native source code, and a CmakeFile.txt file for building
  that source code into a dynamic library.

* `lib`: Contains the Dart code that defines the API of the plugin, and which
  calls into the native code using `dart:ffi`.

* platform folders (`android`, `ios`, `windows`, etc.): Contains the build files
  for building and bundling the native code library with the platform application.

## Binding to native code

To use the native code, bindings in Dart are needed.
To avoid writing these by hand, they are generated from the header file
(`src/flutter_silk.h`) by `package:ffigen`.
Regenerate the bindings by running `flutter pub run ffigen --config ffigen.yaml`.

## Invoking native code

Very native functions can be directly invoked from any isolate.
For example, see `silkToPcm` `pcmToMp3`  `silkToMp3`  in `lib/flutter_silk.dart`.


## Reference

[libSilkCodec](https://github.com/KonataDev/libSilkCodec)
[silk](https://github.com/collects/silk)
[flutter_lame](https://github.com/BestOwl/flutter_lame)
