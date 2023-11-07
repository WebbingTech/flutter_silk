import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';
import 'dart:io';
import 'dart:typed_data';
import 'flutter_silk_bindings_generated.dart';

const String _libName = 'flutter_silk';

typedef _ConvertFn = bool Function(ffi.Pointer<ffi.UnsignedChar>, int, int,
    ffi.Pointer<ffi.Pointer<ffi.UnsignedChar>>, ffi.Pointer<ffi.UnsignedLong>);

/// The dynamic library in which the symbols for [DecoderBindings] can be found.
final ffi.DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return ffi.DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return ffi.DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return ffi.DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final FlutterSilkBindings _bindings = FlutterSilkBindings(_dylib);

Uint8List? silkToMp3(Uint8List silkData)  {
  return _convert(silkData, _bindings.silkToMp3);
}

Uint8List? silkToPcm(Uint8List silkData) {
  return _convert(silkData, _bindings.silkToPcm);
}

Uint8List? pcmToMp3(Uint8List pcmData) {
  return _convert(pcmData, _bindings.pcmToMp3);
}

Uint8List? _convert(Uint8List sourceData, _ConvertFn fn) {
  ffi.Pointer<ffi.UnsignedChar> ffiSourceData =
      calloc<ffi.UnsignedChar>(sourceData.length);
  for (var i = 0; i < sourceData.length; i++) {
    ffiSourceData[i] = sourceData[i];
  }

  var ffiOutputData = calloc<ffi.Pointer<ffi.UnsignedChar>>();
  var outputSize = calloc<ffi.UnsignedLong>();
  var result =
      fn(ffiSourceData, sourceData.length, 16000, ffiOutputData, outputSize);

  Uint8List? output;
  if (result) {
    var outputData = ffiOutputData.value;
    var outputSizeValue = outputSize.value;
    output = Uint8List(outputSizeValue);
    for (var i = 0; i < outputSizeValue; i++) {
      output[i] = outputData[i];
    }
  }

  calloc.free(ffiSourceData);
  calloc.free(ffiOutputData);
  calloc.free(outputSize);

  return output;
}
