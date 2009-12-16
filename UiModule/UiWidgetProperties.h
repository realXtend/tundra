// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiWidgetProperties_h
#define incl_UiModule_UiWidgetProperties_h

#include "UiModuleApi.h"

#include <QtGui>
#include <QtCore>

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
        Q_PROPERTY(QSize my_size_ READ getSize WRITE setSize)
        Q_PROPERTY(QPointF position_ READ getPosition WRITE setPosition)
        Q_PROPERTY(Qt::WindowFlags window_type_ READ getWindowStyle WRITE setWindowStyle)
        Q_PROPERTY(QString widget_name_ READ getWidgetName WRITE setWidgetName)
        Q_PROPERTY(bool show_at_toolbar_ READ isShownAtToolbar WRITE setShowAtToolbar)
        Q_PROPERTY(bool fullscreen_ READ isFullscreen WRITE setFullscreen)
        Q_PROPERTY(AnimationType animation_type_ READ getAnimationType WRITE setAnimationType)

    public:
        //! Constructors
        /*
         * Do not use fullsceen = true if you know what you are doing. Normal module widgets are not fullscreen_ and will have qt made dialog frames around them.
         * Only CoreUi can use layout without going through the normal dialog phase first.
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

        UiWidgetProperties(const QString &widget_name, bool fullscreen, AnimationType animation_type = SlideFromTop, const QSize &size = QSize(200,200))
            : QObject(),
              my_size_(size),
              position_(QPointF(10.0, 60.0)),
              window_type_(Qt::Dialog),
              widget_name_(widget_name),
              show_at_toolbar_(!fullscreen),
              fullscreen_(fullscreen),
              animation_type_(animation_type) {}

        UiWidgetProperties(const QPointF &position = QPointF(10.0, 60.0), const QSize &size = QSize(200,200), const Qt::WindowFlags window_type = Qt::Dialog, const QString &widget_name = QString("UiProxyWidget"), bool show_in_toolbar = true, bool fullscreen = false, AnimationType animation_type = SlideFromTop)
            : QObject(),
              my_size_(size),
              position_(position),
              window_type_(window_type),
              widget_name_(widget_name),
              show_at_toolbar_(show_in_toolbar),
              fullscreen_(fullscreen),
              animation_type_(animation_type) {}

        UiWidgetProperties(const UiWidgetProperties &in_widget_properties)
            : position_(in_widget_properties.getPosition()),
              my_size_(in_widget_properties.getSize()),
              window_type_(in_widget_properties.getWindowStyle()),
              widget_name_(in_widget_properties.getWidgetName()),
              show_at_toolbar_(in_widget_properties.isShownAtToolbar()),
              fullscreen_(in_widget_properties.isFullscreen()),
              animation_type_(in_widget_properties.getAnimationType()) {}

        //! Deconstructor
        ~UiWidgetProperties() {}
        
        //! Setters for properties
        void setSize(const QSize size) { my_size_ = size; }
        void setPosition(const QPointF position) {position_ = position; }
        void setWindowStyle(const Qt::WindowFlags window_type) { window_type_ = window_type; }
        void setWidgetName(const QString &widget_name) { widget_name_ = widget_name; }
        void setShowAtToolbar(const bool show_at_toolbar) { show_at_toolbar_ = show_at_toolbar; }
        void setFullscreen(const bool fullscreen) { fullscreen_ = fullscreen; }
        void setAnimationType(AnimationType animation_type) { animation_type_ = animation_type; }

        //! Getters for properties
        const QSize getSize() const { return my_size_; }
        const QPointF getPosition() const { return position_; }
        const Qt::WindowFlags getWindowStyle() const { return window_type_; }
        const QString getWidgetName() const { return widget_name_; }
        bool isShownAtToolbar() const { return show_at_toolbar_; }
        bool isFullscreen() const { return fullscreen_; }
        AnimationType getAnimationType() const { return animation_type_; }

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
