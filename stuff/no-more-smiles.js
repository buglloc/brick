/*
  Example of script injection in Brick.
  This script replaces all graphical emoticons to their text representation
  Just add it to your config, example (file ~/.config/brick/config.json):
  {
    "client_scripts": [
      "/path/to/no-more-smiles.js"
    ]
  }
*/

if (typeof BX != 'undefined') {
  if (
    BX.MessengerCommon !== void 0
    && BX.MessengerCommon.prepareText !== void 0
    && BX.MessengerCommon._brickPrepareText === void 0
    ) {

    BX.MessengerCommon._brickPrepareText = BX.MessengerCommon.prepareText;
    BX.MessengerCommon._brickSmileCleaner = /<img\s+src=.+?data-code="([^"]+)".+?class="bx-smile"\s*\/>/g;
    BX.MessengerCommon.prepareText = function(text, prepare, quote, image, highlightText) {
      return BX.MessengerCommon._brickPrepareText(
        text.replace(BX.MessengerCommon._brickSmileCleaner, '$1'),
        prepare,
        quote,
        image,
        highlightText
      );
    }
  }
}