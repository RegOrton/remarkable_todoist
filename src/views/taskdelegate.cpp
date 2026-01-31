#include "taskdelegate.h"
#include "../models/taskmodel.h"
#include <QFont>
#include <QPen>

TaskDelegate::TaskDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    painter->save();

    // Fill background (white for e-ink)
    painter->fillRect(option.rect, Qt::white);

    // Get task data from model
    bool completed = index.data(TaskModel::CompletedRole).toBool();
    QString title = index.data(TaskModel::TitleRole).toString();
    QString project = index.data(TaskModel::ProjectRole).toString();
    QString dueDate = index.data(TaskModel::DueDateRole).toString();
    int priority = index.data(TaskModel::PriorityRole).toInt();

    // Draw checkbox
    QRect cbRect = checkboxRect(option.rect);
    drawCheckbox(painter, cbRect, completed);

    // Calculate text area
    QRect txtRect = textRect(option.rect);

    // Split text area into title (top 60%) and metadata (bottom 40%)
    int titleHeight = txtRect.height() * 0.6;
    QRect titleRect(txtRect.left(), txtRect.top(), txtRect.width(), titleHeight);
    QRect metaRect(txtRect.left(), txtRect.top() + titleHeight, txtRect.width(), txtRect.height() - titleHeight);

    // Draw title
    drawTitle(painter, titleRect, title, completed);

    // Draw metadata row
    drawMetadata(painter, metaRect, project, dueDate, priority);

    painter->restore();
}

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    Q_UNUSED(index);
    // Return consistent row height for all items
    // Width will be set by the list view
    return QSize(option.rect.width(), ROW_HEIGHT);
}

void TaskDelegate::drawCheckbox(QPainter *painter, const QRect &rect, bool completed) const
{
    painter->save();

    // High contrast: 2px black border, white fill
    QPen pen(Qt::black);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(Qt::white);

    // Draw checkbox rectangle (no rounded corners for e-ink clarity)
    painter->drawRect(rect.adjusted(1, 1, -1, -1));

    // If completed, draw X inside
    if (completed) {
        pen.setWidth(3);
        painter->setPen(pen);
        // Draw X with some margin from checkbox edges
        int margin = 8;
        QRect innerRect = rect.adjusted(margin, margin, -margin, -margin);
        painter->drawLine(innerRect.topLeft(), innerRect.bottomRight());
        painter->drawLine(innerRect.topRight(), innerRect.bottomLeft());
    }

    painter->restore();
}

void TaskDelegate::drawTitle(QPainter *painter, const QRect &rect, const QString &title, bool completed) const
{
    painter->save();

    // Title font: bold, 16pt for readability on e-ink
    QFont font = painter->font();
    font.setPointSize(TITLE_FONT_SIZE);
    font.setBold(true);
    painter->setFont(font);

    if (completed) {
        // Completed tasks: gray text with strikethrough
        painter->setPen(QColor("#666666"));
        font.setStrikeOut(true);
        painter->setFont(font);
    } else {
        // Active tasks: black text
        painter->setPen(Qt::black);
    }

    // Draw title text, left-aligned, vertically centered
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, title);

    painter->restore();
}

void TaskDelegate::drawMetadata(QPainter *painter, const QRect &rect, const QString &project,
                                const QString &dueDate, int priority) const
{
    painter->save();

    // Metadata font: regular, 12pt
    QFont font = painter->font();
    font.setPointSize(METADATA_FONT_SIZE);
    font.setBold(false);
    font.setStrikeOut(false);
    painter->setFont(font);

    // Dark gray for metadata
    painter->setPen(QColor("#333333"));

    // Build metadata string with separators
    QStringList parts;

    if (!project.isEmpty()) {
        parts << project;
    }

    if (!dueDate.isEmpty()) {
        parts << dueDate;
    }

    // Priority label (P1=highest, P4=lowest)
    // Only show non-default priority (Todoist API: 1=lowest, 4=highest)
    // Priority is already converted in Task::priorityLabel()
    if (priority > 1) {
        // Convert API priority to display: 4->P1, 3->P2, 2->P3, 1->P4
        int displayPriority = 5 - priority;
        QString priorityLabel = QString("P%1").arg(displayPriority);

        // Make priority bold for emphasis
        QFont boldFont = font;
        boldFont.setBold(true);

        // Draw parts before priority normally
        QString beforePriority = parts.join(" | ");
        if (!beforePriority.isEmpty()) {
            beforePriority += " | ";
        }

        // Calculate position for priority label
        QFontMetrics fm(font);
        int beforeWidth = fm.horizontalAdvance(beforePriority);

        // Draw text before priority
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, beforePriority);

        // Draw priority in bold
        painter->setFont(boldFont);
        QRect priorityRect = rect.adjusted(beforeWidth, 0, 0, 0);
        painter->drawText(priorityRect, Qt::AlignLeft | Qt::AlignVCenter, priorityLabel);
    } else {
        // No priority indicator needed, just draw project and date
        QString metaText = parts.join(" | ");
        painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, metaText);
    }

    painter->restore();
}

QRect TaskDelegate::checkboxRect(const QRect &rowRect) const
{
    // Checkbox positioned at left with margin, vertically centered
    int y = rowRect.top() + (rowRect.height() - CHECKBOX_SIZE) / 2;
    return QRect(rowRect.left() + CHECKBOX_MARGIN, y, CHECKBOX_SIZE, CHECKBOX_SIZE);
}

QRect TaskDelegate::textRect(const QRect &rowRect) const
{
    // Text area starts after checkbox + margins, extends to right edge minus margin
    int left = rowRect.left() + CHECKBOX_MARGIN + CHECKBOX_SIZE + TEXT_MARGIN;
    int right = rowRect.right() - TEXT_MARGIN;
    int top = rowRect.top() + 4;  // Small top padding
    int height = rowRect.height() - 8;  // Small bottom padding

    return QRect(left, top, right - left, height);
}
