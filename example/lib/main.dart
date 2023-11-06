import 'dart:io';

import 'package:audioplayers/audioplayers.dart';
import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:flutter_silk/flutter_silk.dart';
import 'package:path/path.dart' as path;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String? inputPath;
  String? outputDir;
  String outputName = "output.mp3";
  bool working = false;
  bool done = false;
  final player = AudioPlayer();

  void selectInputFile() async {
    final result = await FilePicker.platform.pickFiles(
        type: FileType.custom,
        dialogTitle: "Select Silk file",
        allowedExtensions: ["silk"],
        allowMultiple: false);

    if (result == null) {
      return;
    }

    if (result.paths.isEmpty) {
      return;
    }

    setState(() {
      inputPath = result.paths[0];
    });
  }

  void encodeMp3() async {
    if (inputPath == null) {
      throw StateError("inputPath should not be null");
    }

    outputDir = await FilePicker.platform.getDirectoryPath(
        dialogTitle: "Pick a directory to save output MP3 file");
    if (outputDir == null) {
      return;
    }

    setState(() {
      working = true;
    });

    try {
      print("input file: ${inputPath}");
      final silkData = File(inputPath!).readAsBytesSync();

      var output = silkToMp3(silkData);

      final File f = File(path.join(outputDir!, outputName));
      if (output != null ) {
        f.writeAsBytesSync(output!);
        print("Output file: ${f.path}");
      }
      print("Done ${silkData.length}, output -> ${output == null}" );
      setState(() {
        done = true;
      });
    } catch (e) {
      showDialog(
          context: context,
          builder: (context) => AlertDialog(
            title: const Text("Error"),
            content: Text(e.toString()),
          ));
    } finally {
      setState(() {
        working = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    const textStyle = TextStyle(fontSize: 25);
    const spacerSmall = SizedBox(height: 10);
    const spacerLarge = SizedBox(height: 30);
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Flutter SILK & LAME Example'),
        ),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: Column(
              children: [
                const Text(
                  'Call SILK/LAME API through FFI that is shipped as source in the package. '
                      'The native code is built as part of the Flutter Runner build.',
                  style: textStyle,
                  textAlign: TextAlign.center,
                ),
                const Divider(),
                spacerLarge,
                ElevatedButton(
                    onPressed: !working ? selectInputFile : null,
                    child: const Text(
                      "Select Silk file",
                      style: textStyle,
                    )),
                spacerSmall,
                RichText(
                  text: TextSpan(
                      style: const TextStyle(fontSize: 25, color: Colors.black),
                      children: [
                        const TextSpan(
                            text: "Input Silk file: ",
                            style: TextStyle(fontWeight: FontWeight.bold)),
                        TextSpan(text: inputPath)
                      ]),
                  textAlign: TextAlign.center,
                ),
                spacerSmall,
                TextFormField(
                    onChanged: (v) => setState(() {
                      outputName = v;
                    }),
                    decoration:
                    const InputDecoration(labelText: "Output MP3 filename"),
                    initialValue: outputName),
                spacerSmall,
                ElevatedButton(
                    onPressed:
                    inputPath != null && outputName.isNotEmpty && !working
                        ? encodeMp3
                        : null,
                    child: const Text(
                      "Encode to MP3",
                      style: textStyle,
                    )),
                spacerSmall,
                ElevatedButton(
                  child: const Text('Play',
                    style: textStyle,),
                  onPressed: () async {
                      done ? await player.play(DeviceFileSource(path.join(outputDir!, outputName))) : null;
                  },
                ),
                spacerSmall,
                if (working) const CircularProgressIndicator(),
              ],
            ),
          ),
        ),
      ),
    );
  }
}