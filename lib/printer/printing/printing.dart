import 'package:method_channel/printer/pdf/widgets.dart';

import 'src/fonts/gfonts.dart';

export 'src/asset_utils.dart';
export 'src/cache.dart';
export 'src/callback.dart';
export 'src/fonts/gfonts.dart';
export 'src/preview/actions.dart';
export 'src/preview/pdf_preview.dart';
export 'src/printer.dart';
export 'src/printing.dart';
export 'src/printing_info.dart';
export 'src/raster.dart';
export 'src/widget_wrapper.dart';

Future<void> pdfDefaultTheme() async {
  if (ThemeData.buildThemeData != null) {
    return;
  }

  final base = await PdfGoogleFonts.openSansRegular();
  final bold = await PdfGoogleFonts.openSansBold();
  final italic = await PdfGoogleFonts.openSansItalic();
  final boldItalic = await PdfGoogleFonts.openSansBoldItalic();
  final emoji = await PdfGoogleFonts.notoColorEmoji();
  final icons = await PdfGoogleFonts.materialIcons();

  ThemeData.buildThemeData = () {
    return ThemeData.withFont(
      base: base,
      bold: bold,
      italic: italic,
      boldItalic: boldItalic,
      icons: icons,
      fontFallback: [emoji, base],
    );
  };
}
