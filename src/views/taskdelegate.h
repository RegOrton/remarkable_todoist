#ifndef TASKDELEGATE_H
#define TASKDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QStyleOptionViewItem>

/**
 * TaskDelegate - Custom item delegate for rendering task rows
 *
 * Renders task rows optimized for e-ink display:
 * - High contrast: black text on white background
 * - Large checkbox (48x48) for stylus input
 * - Two-line layout: title on top, metadata below
 * - 80px row height for adequate touch targets
 *
 * Layout:
 * +--------------------------------------------------+
 * | [checkbox 48x48]  Task Title (bold, 16pt)        |
 * |                   Project | Due Date | P1        |
 * +--------------------------------------------------+
 */
class TaskDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit TaskDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

signals:
    void taskCheckboxToggled(int row);

private:
    // Layout constants optimized for e-ink and stylus input
    static const int ROW_HEIGHT = 80;
    static const int CHECKBOX_SIZE = 48;
    static const int CHECKBOX_MARGIN = 12;
    static const int TEXT_MARGIN = 12;
    static const int TITLE_FONT_SIZE = 16;
    static const int METADATA_FONT_SIZE = 12;

    // Helper methods for drawing
    void drawCheckbox(QPainter *painter, const QRect &rect, bool completed) const;
    void drawTitle(QPainter *painter, const QRect &rect, const QString &title, bool completed) const;
    void drawMetadata(QPainter *painter, const QRect &rect, const QString &project,
                      const QString &dueDate, int priority) const;

    // Calculate checkbox rect within row
    QRect checkboxRect(const QRect &rowRect) const;

    // Calculate text area rect (to the right of checkbox)
    QRect textRect(const QRect &rowRect) const;
};

#endif // TASKDELEGATE_H
