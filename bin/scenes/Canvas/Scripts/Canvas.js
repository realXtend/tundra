// Canvas.js - Creates unique texture name for the GraphicsViewCanvas' output texture

if (me.graphicsViewCanvas)
    me.graphicsViewCanvas.outputTexture = asset.GenerateUniqueAssetName("Texture", "Canvas");
