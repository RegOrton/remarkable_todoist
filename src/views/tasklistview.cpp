#include "tasklistview.h"
#include "taskdelegate.h"

TaskListView::TaskListView(QWidget *parent)
    : QListView(parent)
    , m_delegate(new TaskDelegate(this))
    , m_partialRefreshCount(0)
{
    // Set custom delegate for task row rendering
    setItemDelegate(m_delegate);

    setupAppearance();
    setupScrolling();
    applyStyleSheet();
}

TaskListView::~TaskListView()
{
    // m_delegate is parented to this, will be deleted automatically
}

void TaskListView::setupAppearance()
{
    // High contrast for e-ink
    setFrameStyle(QFrame::NoFrame);

    // No alternating row colors - keep simple for e-ink
    setAlternatingRowColors(false);

    // Single selection mode
    setSelectionMode(QAbstractItemView::SingleSelection);

    // No editing via delegate (tasks edited through other means)
    setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Uniform item sizes for performance (all rows are same height)
    setUniformItemSizes(true);
}

void TaskListView::setupScrolling()
{
    // Scroll per pixel for smooth scrolling
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // No horizontal scrollbar - full width layout
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Vertical scrollbar only when needed
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Word wrap disabled - titles truncate at edge
    setWordWrap(false);
}

void TaskListView::applyStyleSheet()
{
    // High contrast styling optimized for e-ink
    setStyleSheet(R"(
        QListView {
            background-color: white;
            border: none;
        }
        QListView::item {
            border-bottom: 1px solid #cccccc;
            padding: 0px;
        }
        QListView::item:selected {
            background-color: #e0e0e0;
        }
        QScrollBar:vertical {
            width: 20px;
            background: #f0f0f0;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #888888;
            min-height: 40px;
            border-radius: 4px;
            margin: 2px;
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )");
}

void TaskListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                const QVector<int> &roles)
{
    // Call parent implementation first
    QListView::dataChanged(topLeft, bottomRight, roles);

    // Track partial refreshes for e-ink optimization
    m_partialRefreshCount++;

    if (m_partialRefreshCount >= MAX_PARTIAL_REFRESHES) {
        triggerFullRefresh();
    }
}

void TaskListView::triggerFullRefresh()
{
    // Reset counter
    m_partialRefreshCount = 0;

    // Force full widget repaint
    // On reMarkable 2 with rm2fb, this triggers a full e-ink refresh
    // which clears ghosting from accumulated partial updates
    repaint();

    // Note: For actual rm2fb integration, additional ioctl calls
    // may be needed to explicitly request GC16 waveform.
    // For Phase 1, standard Qt repaint is adequate as rm2fb
    // intercepts and handles waveform selection.
}
