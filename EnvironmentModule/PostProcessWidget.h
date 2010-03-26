// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OgreRenderingModule_PostProcessWidget_h
#define incl_OgreRenderingModule_PostProcessWidget_h

#include "ui_postprocesswidget.h"

#include <QWidget>
#include <QCheckBox>

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
        //! \param effect 
        PostProcessWidget(std::vector<std::string> &effects);

        //! Destructor.
        virtual ~PostProcessWidget();

        //! Widget adds itself to the scene
        void AddSelfToScene(EnvironmentModule *env_module);

        //! Adds handler to handle the postprocess requests. Note that this class doesn't own the handler and is not responsible for deleting it
        void AddHandler(OgreRenderer::CompositionHandler *handler);

        //! Add effect names that will be shown in dialog with radiobuttons
        void AddEffects(std::vector<std::string> &effects);

        //! Set effectbutton checked/unchecked. Used if effects are turned on/off from somewhere else
        void EnableEffect(const std::string &effect_name, bool enable);

        //! Disables all effects
        void DisableAllEffects();

    public slots:
        void HandleSelection(bool checked, const QString &name);

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

    public slots:
        void ButtonToggled(bool checked);

    signals:
        void Toggled(bool checked, const QString &name);
    };
}

#endif
