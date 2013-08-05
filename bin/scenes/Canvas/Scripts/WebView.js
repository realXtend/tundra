/*  For conditions of distribution and use, see copyright notice in LICENSE

    WebView.js - Shows web page using QGraphicsWebView on GraphicsViewCanvas */

var uiWidget = null;

if (me.graphicsViewCanvas)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
    engine.ImportExtension("qt.webkit");

    var webView = new QGraphicsWebView();
    webView.url = new QUrl("http://realxtend.wordpress.com/");
    webView.size = new QSize(me.graphicsViewCanvas.width, me.graphicsViewCanvas.height);
    me.graphicsViewCanvas.GraphicsScene().addItem(webView); // Note: for graphics items we use addItem instead of addWidget.
}
else
{
    console.LogError("GraphicsViewCanvas component not available!");
}

function OnScriptDestroyed()
{
    if (framework.IsExiting())
        return; // Application shutting down, the widget pointers are garbage.
    if (webView)
    {
        webView.deleteLater();
        webView = null;
    }
}
