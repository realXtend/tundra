// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_PostProcessWidget_h
#define incl_OgreRenderingModule_PostProcessWidget_h

#include "ui_postprocesswidget.h"

#include <QWidget>
#include <QCheckBox>
#include <QVector>
#include <QString>

namespace OgreRenderer
{
    class CompositionHandler;
}

namespace Environment
{
    class EnvironmentModule;

    //! Dialog for postprocessing effects
    //! \ingroup EnvironmentModuleClient.
    class PostProcessWidget : public QWidget
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! \param effects List of effect names
        explicit PostProcessWidget(QVector<QString> &effects);

        //! Destructor.
        virtual ~PostProcessWidget();

        //! Sets handler to handle the postprocess requests. Note that this class doesn't own the handler and is not responsible for deleting it
        void SetHandler(OgreRenderer::CompositionHandler *handler);

        //! Add effect names that will be shown in dialog with radiobuttons
        void AddEffects(QVector<QString> &effects);

        //! Set effectbutton checked/unchecked. Used if effects are turned on/off from somewhere else
        void EnableEffect(const QString &effect_name, bool enable);

        //! Disables all effects
        void DisableAllEffects();

    public slots:
        void HandleSelection(bool checked, const QString &name);

    protected:
        void changeEvent(QEvent* e);

    private:
        //! Widget that this QWidget contains
        Ui::SelectionWidget widget_;

        //! CompositionHandler that is notified when radiobutton is clicked
        OgreRenderer::CompositionHandler *handler_;
    };

    //! inherited to get name of the checkbox with toggle signal
    class NamedCheckBox: public QCheckBox
    {
        Q_OBJECT

    public:
        NamedCheckBox(const QString &text, QWidget *parent = 0);
        virtual ~NamedCheckBox();

    protected:
        void changeEvent(QEvent* e);

    public slots:
        void ButtonToggled(bool checked);

    signals:
        void Toggled(bool checked, const QString &name);
    };
}

#endif
