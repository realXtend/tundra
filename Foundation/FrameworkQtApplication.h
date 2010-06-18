// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_FrameworkQtApplication_h
#define incl_Foundation_FrameworkQtApplication_h

#include "ForwardDefines.h"

#include <QTimer>
#include <QApplication>
#include <QStringList>

class QDir;
class QGraphicsView;
class QTranslator;

namespace Foundation
{
    class Framework;

    /// Bridges QtApplication and Framework objects together and helps drive the continuous
    /// application update ticks using a timer object.
    /// Owns QApplication and QGraphicsView (main window) for its lifetime.

    class FrameworkQtApplication : public QApplication
    {
        Q_OBJECT

    public:
        FrameworkQtApplication (Framework *owner, int &argc, char** argv);
        ~FrameworkQtApplication();

        QGraphicsView *GetUIView() const;
        void SetUIView (std::auto_ptr <QGraphicsView> view);

        virtual bool notify(QObject* receiver, QEvent* e);

        void Go();

    public slots:
        void UpdateFrame();
        void ChangeLanguage(const QString& file);
        void AboutToExit();

    signals:
        /// Signal is sent when QApplication language is changed, provided for convience.
        void LanguageChanged();
        
        /// Signal is sent when window close / framework exit has been requested
        void ExitRequested();

    protected:
        bool eventFilter (QObject *obj, QEvent *event);

    private:
        QStringList GetQmFiles(const QDir& dir);

        Framework *framework_;
        QTimer frame_update_timer_;
        bool app_activated_;

        std::auto_ptr <QGraphicsView> view_;
        QTranslator* native_translator_;
        QTranslator* app_translator_;
    };

}

#endif
