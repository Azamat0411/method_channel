import 'dart:async';
import 'dart:io' show Platform;
import 'package:flutter/foundation.dart' show kIsWeb;

import 'package:flutter/material.dart';
import 'package:method_channel/text_to_speech/flutter_tts.dart';

void main() => runApp(MyApp());

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

enum TtsState { playing, stopped, paused, continued }

class _MyAppState extends State<MyApp> {
  late FlutterTts flutterTts;
  String? language;
  String? engine;
  double volume = 0.5;
  double pitch = 1.0;
  double rate = 0.5;
  bool isCurrentLanguageInstalled = false;

  String? _newVoiceText;
  int? _inputLength;

  TtsState ttsState = TtsState.stopped;

  get isPlaying => ttsState == TtsState.playing;
  get isStopped => ttsState == TtsState.stopped;
  get isPaused => ttsState == TtsState.paused;
  get isContinued => ttsState == TtsState.continued;

  bool get isIOS => !kIsWeb && Platform.isIOS;
  bool get isAndroid => !kIsWeb && Platform.isAndroid;
  bool get isWindows => !kIsWeb && Platform.isWindows;
  bool get isWeb => kIsWeb;

  @override
  initState() {
    super.initState();
    initTts();
  }

  initTts() {
    flutterTts = FlutterTts();

    _setAwaitOptions();

    if (isAndroid) {
      _getDefaultEngine();
    }

    flutterTts.setStartHandler(() {
      setState(() {
        print("Playing");
        ttsState = TtsState.playing;
      });
    });

    flutterTts.setCompletionHandler(() {
      setState(() {
        print("Complete");
        ttsState = TtsState.stopped;
      });
    });

    flutterTts.setCancelHandler(() {
      setState(() {
        print("Cancel");
        ttsState = TtsState.stopped;
      });
    });

    if (isWeb || isIOS || isWindows) {
      flutterTts.setPauseHandler(() {
        setState(() {
          print("Paused");
          ttsState = TtsState.paused;
        });
      });

      flutterTts.setContinueHandler(() {
        setState(() {
          print("Continued");
          ttsState = TtsState.continued;
        });
      });
    }

    flutterTts.setErrorHandler((msg) {
      setState(() {
        print("error: $msg");
        ttsState = TtsState.stopped;
      });
    });
  }

  Future<dynamic> _getLanguages() => flutterTts.getLanguages;

  Future<dynamic> _getEngines() => flutterTts.getEngines;

  Future _getDefaultEngine() async {
    var engine = await flutterTts.getDefaultEngine;
    if (engine != null) {
      print(engine);
    }
  }

  Future _speak() async {
    await flutterTts.setVolume(volume);
    await flutterTts.setSpeechRate(rate);
    await flutterTts.setPitch(pitch);

    if (_newVoiceText != null) {
      if (_newVoiceText!.isNotEmpty) {
        await flutterTts.speak(_newVoiceText!);
      }
    }
  }

  Future _setAwaitOptions() async {
    await flutterTts.awaitSpeakCompletion(true);
  }

  Future _stop() async {
    var result = await flutterTts.stop();
    if (result == 1) setState(() => ttsState = TtsState.stopped);
  }

  Future _pause() async {
    var result = await flutterTts.pause();
    if (result == 1) setState(() => ttsState = TtsState.paused);
  }

  @override
  void dispose() {
    super.dispose();
    flutterTts.stop();
  }

  List<DropdownMenuItem<String>> getEnginesDropDownMenuItems(dynamic engines) {
    var items = <DropdownMenuItem<String>>[];
    for (dynamic type in engines) {
      items.add(DropdownMenuItem(
          value: type as String?, child: Text(type as String)));
    }
    return items;
  }

  void changedEnginesDropDownItem(String? selectedEngine) {
    flutterTts.setEngine(selectedEngine!);
    language = null;
    setState(() {
      engine = selectedEngine;
    });
  }

  List<DropdownMenuItem<String>> getLanguageDropDownMenuItems(
      dynamic languages) {
    var items = <DropdownMenuItem<String>>[];
    for (dynamic type in languages) {
      items.add(DropdownMenuItem(
          value: type as String?, child: Text(type as String)));
    }
    return items;
  }

  void changedLanguageDropDownItem(String? selectedType) {
    setState(() {
      language = selectedType;
      flutterTts.setLanguage(language!);
      if (isAndroid) {
        flutterTts
            .isLanguageInstalled(language!)
            .then((value) => isCurrentLanguageInstalled = (value as bool));
      }
    });
  }

  void _onChange(String text) {
    setState(() {
      _newVoiceText = text;
    });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: Text('Flutter TTS'),
        ),
        body: SingleChildScrollView(
          scrollDirection: Axis.vertical,
          child: Column(
            children: [
              _inputSection(),
              _btnSection(),
              _engineSection(),
              _futureBuilder(),
              _buildSliders(),
              if (isAndroid) _getMaxSpeechInputLengthSection(),
            ],
          ),
        ),
      ),
    );
  }

  Widget _engineSection() {
    if (isAndroid) {
      return FutureBuilder<dynamic>(
          future: _getEngines(),
          builder: (BuildContext context, AsyncSnapshot<dynamic> snapshot) {
            if (snapshot.hasData) {
              return _enginesDropDownSection(snapshot.data);
            } else if (snapshot.hasError) {
              return Text('Error loading engines...');
            } else
              return Text('Loading engines...');
          });
    } else
      return Container(width: 0, height: 0);
  }

  Widget _futureBuilder() => FutureBuilder<dynamic>(
      future: _getLanguages(),
      builder: (BuildContext context, AsyncSnapshot<dynamic> snapshot) {
        if (snapshot.hasData) {
          return _languageDropDownSection(snapshot.data);
        } else if (snapshot.hasError) {
          return Text('Error loading languages...');
        } else
          return Text('Loading Languages...');
      });

  Widget _inputSection() => Container(
      alignment: Alignment.topCenter,
      padding: EdgeInsets.only(top: 25.0, left: 25.0, right: 25.0),
      child: TextField(
        onChanged: (String value) {
          _onChange(value);
        },
      ));

  Widget _btnSection() {
    if (isAndroid) {
      return Container(
          padding: EdgeInsets.only(top: 50.0),
          child:
          Row(mainAxisAlignment: MainAxisAlignment.spaceEvenly, children: [
            _buildButtonColumn(Colors.green, Colors.greenAccent,
                Icons.play_arrow, 'PLAY', _speak),
            _buildButtonColumn(
                Colors.red, Colors.redAccent, Icons.stop, 'STOP', _stop),
          ]));
    } else {
      return Container(
          padding: EdgeInsets.only(top: 50.0),
          child:
          Row(mainAxisAlignment: MainAxisAlignment.spaceEvenly, children: [
            _buildButtonColumn(Colors.green, Colors.greenAccent,
                Icons.play_arrow, 'PLAY', _speak),
            _buildButtonColumn(
                Colors.red, Colors.redAccent, Icons.stop, 'STOP', _stop),
            _buildButtonColumn(
                Colors.blue, Colors.blueAccent, Icons.pause, 'PAUSE', _pause),
          ]));
    }
  }

  Widget _enginesDropDownSection(dynamic engines) => Container(
    padding: EdgeInsets.only(top: 50.0),
    child: DropdownButton(
      value: engine,
      items: getEnginesDropDownMenuItems(engines),
      onChanged: changedEnginesDropDownItem,
    ),
  );

  Widget _languageDropDownSection(dynamic languages) => Container(
      padding: EdgeInsets.only(top: 10.0),
      child: Row(mainAxisAlignment: MainAxisAlignment.center, children: [
        DropdownButton(
          value: language,
          items: getLanguageDropDownMenuItems(languages),
          onChanged: changedLanguageDropDownItem,
        ),
        Visibility(
          visible: isAndroid,
          child: Text("Is installed: $isCurrentLanguageInstalled"),
        ),
      ]));

  Column _buildButtonColumn(Color color, Color splashColor, IconData icon,
      String label, Function func) {
    return Column(
        mainAxisSize: MainAxisSize.min,
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          IconButton(
              icon: Icon(icon),
              color: color,
              splashColor: splashColor,
              onPressed: () => func()),
          Container(
              margin: const EdgeInsets.only(top: 8.0),
              child: Text(label,
                  style: TextStyle(
                      fontSize: 12.0,
                      fontWeight: FontWeight.w400,
                      color: color)))
        ]);
  }

  Widget _getMaxSpeechInputLengthSection() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      children: [
        ElevatedButton(
          child: Text('Get max speech input length'),
          onPressed: () async {
            _inputLength = await flutterTts.getMaxSpeechInputLength;
            setState(() {});
          },
        ),
        Text("$_inputLength characters"),
      ],
    );
  }

  Widget _buildSliders() {
    return Column(
      children: [_volume(), _pitch(), _rate()],
    );
  }

  Widget _volume() {
    return Slider(
        value: volume,
        onChanged: (newVolume) {
          setState(() => volume = newVolume);
        },
        min: 0.0,
        max: 1.0,
        divisions: 10,
        label: "Volume: $volume");
  }

  Widget _pitch() {
    return Slider(
      value: pitch,
      onChanged: (newPitch) {
        setState(() => pitch = newPitch);
      },
      min: 0.5,
      max: 2.0,
      divisions: 15,
      label: "Pitch: $pitch",
      activeColor: Colors.red,
    );
  }

  Widget _rate() {
    return Slider(
      value: rate,
      onChanged: (newRate) {
        setState(() => rate = newRate);
      },
      min: 0.0,
      max: 1.0,
      divisions: 10,
      label: "Rate: $rate",
      activeColor: Colors.green,
    );
  }
}


/*
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:method_channel/printer/printing/printing.dart';
import 'package:method_channel/printer/pdf/pdf.dart';
import 'package:method_channel/printer/pdf/widgets.dart' as pw;
import 'package:method_channel/printer/pdf/src/widgets/font.dart';
import 'package:method_channel/printer/printing/src/interface.dart';
import 'package:method_channel/printer/printing/src/preview/controller.dart';

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const MyApp('Printing Demo'));
}

class MyApp extends StatelessWidget {
  const MyApp(this.title, {Key? key}) : super(key: key);

  final String title;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text(title)),
        body: Container(),
        floatingActionButton: FloatingActionButton(
          onPressed: () async {
            debugPrint('aaaaaa');
            Future<Uint8List> onLayout(format) => _generatePdf(const PdfPageFormat(70 * 72 / 25.4, 200 * 72 / 25.4, marginAll: 5).portrait, "");
            PrintingPlatform.instance.layoutPdf(
              null,
              onLayout,
              'name',
              const PdfPageFormat(70 * 72 / 25.4, 200 * 72 / 25.4, marginAll: 5).portrait,
              true,
              false
            );
            // debugPrint("result: $result");
          },
        ),
      ),
    );
  }

  Future<Uint8List> _generatePdf(PdfPageFormat format, String title) async {
    final pdf = pw.Document(version: PdfVersion.pdf_1_5, compress: true);

    pdf.addPage(
      pw.Page(
        pageFormat: format,
        build: (context) {
          return pw.Column(
            children: [
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('AutoMed',
                      textAlign: pw.TextAlign.center,
                      style: pw.TextStyle(
                        fontSize: 25.0,
                        fontWeight: pw.FontWeight.bold,
                        fontItalic: Font.helvetica(),
                      ))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('AutoMed M.CH.J',
                      textAlign: pw.TextAlign.center,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.bold))),
              pw.Divider(),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('Manzil: Yangi Sergeli 14 A',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10, fontWeight: pw.FontWeight.bold))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('+998(90) 191-66-00',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10, fontWeight: pw.FontWeight.normal))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('+998(90) 191-66-00',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.normal))),
              pw.Divider(),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('30.07.2022 15:18',
                      textAlign: pw.TextAlign.center,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.bold))),
              pw.Divider(),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text(
                      '1. MANNOL TOTAL ECS 5w-30 - 1 * 10 200 = 10 200',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.normal))),
              pw.Divider(),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('QQS summa: 35 678',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.normal))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('Summa: 35 678',
                      textAlign: pw.TextAlign.left,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.bold))),
              pw.Divider(),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('Haridingiz uchun rahmat',
                      textAlign: pw.TextAlign.right,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.bold))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text('Mijoz Azamat',
                      textAlign: pw.TextAlign.right,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.normal))),
              pw.SizedBox(
                  width: double.infinity,
                  child: pw.Text("Xizmat ko'rsatdi Azamat",
                      textAlign: pw.TextAlign.right,
                      style: pw.TextStyle(
                          fontSize: 10.0, fontWeight: pw.FontWeight.normal)))
            ],
          );
        },
      ),
    );

    return pdf.save();
  }
}
 */