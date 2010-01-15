// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiWidgetProperties_h
#define incl_UiModule_UiWidgetProperties_h

#include "UiModuleApi.h"

#include <QObject>
#include <QSize>
#include <QPointF>
#include <QString>

namespace UiServices
{
    enum AnimationType
    {
        SlideFromTop,
        SlideFromBottom
    };

    /*************** WidgetProperties : QObject ***************/

    class UI_MODULE_API UiWidgetProperties : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(QSize my_size_ READ GetSize WRITE SetSize)
        Q_PROPERTY(QPointF position_ READ GetPosition WRITE SetPosition)
        Q_PROPERTY(Qt::WindowFlags window_type_ READ GetWindowStyle WRITE SetWindowStyle)
        Q_PROPERTY(QString widget_name_ READ GetWidgetName WRITE SetWidgetName)
        Q_PROPERTY(bool show_at_toolbar_ READ IsShownAtToolbar WRITE SetShowAtToolbar)
        Q_PROPERTY(bool fullscreen_ READ IsFullscreen WRITE SetFullscreen)
        Q_PROPERTY(AnimationType animation_type_ READ GetAnimationType WRITE SetAnimationType)

    public:
        //! Constructors
        /*
         * Do not use fullsceen = true if you know what you are doing. Normal module widgets are not fullscreen_ and will
         * have qt made dialog frames around them. Only CoreUi can use layout without going through the normal dialog phase first.
         */
        UiWidgetProperties(const QString &widget_name, AnimationType animation_type = SlideFromTop, const QSize &size = QSize(200,200))
            : QObject(),
              my_size_(size),
              position_(QPointF(10.0, 60.0)),
              window_type_(Qt::Dialog),
              widget_name_(widget_name),
              show_at_toolbar_(true),
              fullscreen_(false),
              animation_type_(animation_type) {}

        UiWidgetProperties(const QString &widget_name, bool core_layout_widget, AnimationType animation_type = SlideFromTop,
            const QSize &size = QSize(200,200))
            : QObject(),
              my_size_(size),
              position_(QPointF(10.0, 60.0)),
              window_type_(0),
              widget_name_(widget_name),
              show_at_toolbar_(!core_layout_widget),
              fullscreen_(core_layout_widget),
              animation_type_(animation_type) 
        {
            if (core_layout_widget)
                window_type_ = Qt::Widget;
            else
                window_type_ = Qt::Dialog;
        }

        UiWidgetProperties(const QPointF &position = QPointF(10.0, 60.0), const QSize &size = QSize(200,200),
            const Qt::WindowFlags window_type = Qt::Dialog, const QString &widget_name = QString("UiProxyWidget"),
            bool show_in_toolbar = true, bool fullscreen = false, AnimationType animation_type = SlideFromTop)
            : QObject(),
              my_size_(size),
              position_(position),
              window_type_(window_type),
              widget_name_(widget_name),
              show_at_toolbar_(show_in_toolbar),
              fullscreen_(fullscreen),
              animation_type_(animation_type) {}

        UiWidgetProperties(const UiWidgetProperties &in_widget_properties)
            : position_(in_widget_properties.GetPosition()),
              my_size_(in_widget_properties.GetSize()),
              window_type_(in_widget_properties.GetWindowStyle()),
              widget_name_(in_widget_properties.GetWidgetName()),
              show_at_toolbar_(in_widget_properties.IsShownAtToolbar()),
              fullscreen_(in_widget_properties.IsFullscreen()),
              animation_type_(in_widget_properties.GetAnimationType()) {}

        //! Deconstructor
        ~UiWidgetProperties() {}

        //! Setters for properties
        void SetSize(const QSize &size) { my_size_ = size; }
        void SetPosition(const QPointF &position) {position_ = position; }
        void SetWindowStyle(const Qt::WindowFlags &window_type) { window_type_ = window_type; }
        void SetWidgetName(const QString &widget_name) { widget_name_ = widget_name; }
        void SetShowAtToolbar(const bool &show_at_toolbar) { show_at_toolbar_ = show_at_toolbar; }
        void SetFullscreen(const bool &fullscreen) { fullscreen_ = fullscreen; }
        void SetAnimationType(AnimationType animation_type) { animation_type_ = animation_type; }

        //! Getters for properties
        const QSize GetSize() const { return my_size_; }
        const QPointF GetPosition() const { return position_; }
        const Qt::WindowFlags GetWindowStyle() const { return window_type_; }
        const QString GetWidgetName() const { return widget_name_; }
        bool IsShownAtToolbar() const { return show_at_toolbar_; }
        bool IsFullscreen() const { return fullscreen_; }
        AnimationType GetAnimationType() const { return animation_type_; }

    private:
        QSize my_size_;
        QPointF position_;
        Qt::WindowFlags window_type_;
        QString widget_name_;
        bool show_at_toolbar_;
        bool fullscreen_;
        AnimationType animation_type_;
    };
}

#endif // incl_UiModule_UiWidgetProperties_h
