# Billboard Hit Testing

## Overview

The `BillboardDrawable` class now supports hit testing to detect mouse clicks on billboards. This is useful for making billboards interactive.

## How It Works

1. **Screen Position Calculation**: The `updateScreenPositions()` method transforms 3D billboard positions to 2D screen coordinates using the view and projection matrices.

2. **Hit Testing**: The `hitTest()` method checks if a screen position (e.g., mouse cursor) intersects with any billboard.

## Usage Example

### In GLWidget (or similar class with mouse events):

```cpp
void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    // ... existing code ...

    // Update billboard screen positions (only when needed, e.g., on hover)
    if (m_heightmapGridDrawer.billboardDrawable()) {
        m_heightmapGridDrawer.billboardDrawable()->updateScreenPositions(
            m_viewMatrix,
            m_projectionMatrix,
            size()
        );

        // Check if mouse is over any billboard
        int hitIndex = m_heightmapGridDrawer.billboardDrawable()->hitTest(event->pos());
        if (hitIndex >= 0) {
            // Billboard at index hitIndex is under the cursor
            qDebug() << "Billboard" << hitIndex << "is under cursor";
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    // ... existing code ...

    // Handle click on billboard
    if (event->button() == Qt::LeftButton) {
        if (m_heightmapGridDrawer.billboardDrawable()) {
            m_heightmapGridDrawer.billboardDrawable()->updateScreenPositions(
                m_viewMatrix,
                m_projectionMatrix,
                size()
            );

            int hitIndex = m_heightmapGridDrawer.billboardDrawable()->hitTest(event->pos());
            if (hitIndex >= 0) {
                // Handle billboard click
                handleBillboardClick(hitIndex);
                return;  // Don't process other mouse events
            }
        }
    }

    // ... existing mouse press handling ...
}
```

## Performance Considerations

- Call `updateScreenPositions()` only when needed (e.g., on mouse movement when hovering over the 3D view)
- Don't call it every frame during rendering - it's designed to be called "on demand"
- The calculation is relatively fast, but avoid calling it unnecessarily

## API Reference

### `void updateScreenPositions(const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix, const QSize& viewportSize)`

Calculates screen positions for all billboards. Must be called before `hitTest()`.

**Parameters:**
- `viewMatrix`: The current view matrix from the camera
- `projectionMatrix`: The current projection matrix
- `viewportSize`: The size of the viewport (usually from `widget->size()`)

### `int hitTest(const QPoint& screenPos) const`

Tests if a screen position intersects with any billboard.

**Parameters:**
- `screenPos`: Screen position to test (e.g., from `QMouseEvent::pos()`)

**Returns:**
- Index of the billboard in the internal array (same order as added with `addBillboard()`)
- Returns `-1` if no billboard was hit

### `const QVector<BillboardScreenPosition>& screenPositions() const`

Returns the cached screen positions. Useful for debugging or advanced use cases.

**Returns:**
- Vector of `BillboardScreenPosition` structures containing:
  - `screenPos`: Center position on screen
  - `screenSize`: Size in screen pixels
  - `billboardIndex`: Original index in the billboard array
  - `depth`: Z-depth (for sorting)
