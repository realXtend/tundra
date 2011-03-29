// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_NaaliApplication_h
#define incl_Foundation_NaaliApplication_h

//#include "ForwardDefines.h"

#include <QTimer>
#include <QApplication>
#include <QStringList>

class QDir;
class QGraphicsView;
class QTranslator;

namespace Foundation
{
    class Framework;

    /// Represents the Naali-subclassed instance of the Qt's QApplication singleton.
    class NaaliApplication : public QApplication
    {
        Q_OBJECT

    public:
        /// Constructor.
        /** Qt requires its own data copy of the argc/argv parameters, so NaaliApplication
            caches them. Pass in received command-line parameters here.
            @param owner Pass in the root framework pointer here.
        */
        NaaliApplication(Framework *owner, int &argc, char **argv);

        ~NaaliApplication();

        virtual bool notify(QObject *receiver, QEvent *e);

        void Go();

    public slots:
        void UpdateFrame();
        void ChangeLanguage(const QString& file);
        void AboutToExit();

    signals:
        /// This signal is sent when QApplication language is changed, provided for convenience.
        void LanguageChanged();

        /// This signal is sent when the main window is closed or framework exit has been requested.
        void ExitRequested();

    protected:
        bool eventFilter(QObject *obj, QEvent *event);

    private:
        QStringList GetQmFiles(const QDir &dir);
        Framework *framework;
        QTimer frameUpdateTimer;
        bool appActivated;
        QTranslator *nativeTranslator;
        QTranslator *appTranslator;
        int argc; ///< Command line argument count as supplied by the operating system.
        char **argv; ///< Command line arguments as supplied by the operating system.
    };
}

#endif
