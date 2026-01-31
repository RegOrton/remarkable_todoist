#ifndef TASKLISTVIEW_H
#define TASKLISTVIEW_H

#include <QListView>
#include <QVector>

class TaskDelegate;

/**
 * TaskListView - Scrollable task list optimized for e-ink display
 *
 * Configures QListView for reMarkable 2 e-ink display:
 * - Vertical scrolling with touch-friendly scrollbar
 * - High contrast styling (black on white)
 * - No horizontal scroll (full-width layout)
 * - E-ink refresh tracking infrastructure
 *
 * Uses TaskDelegate for custom row rendering.
 */
class TaskListView : public QListView
{
    Q_OBJECT

public:
    explicit TaskListView(QWidget *parent = nullptr);
    ~TaskListView();

    /**
     * Trigger a full screen refresh
     * Used to clear e-ink ghosting after multiple partial updates
     */
    void triggerFullRefresh();

protected:
    /**
     * Override to track partial refreshes
     * After MAX_PARTIAL_REFRESHES, triggers full refresh to clear ghosting
     */
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles = QVector<int>()) override;

private:
    void setupAppearance();
    void setupScrolling();
    void applyStyleSheet();

    TaskDelegate *m_delegate;

    // E-ink refresh management
    int m_partialRefreshCount;
    static const int MAX_PARTIAL_REFRESHES = 5;
};

#endif // TASKLISTVIEW_H
