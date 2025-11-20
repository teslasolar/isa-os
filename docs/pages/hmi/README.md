# ISA-OS HMI System - Perspective Architecture

## Overview

The ISA-OS HMI (Human-Machine Interface) system is a web-based, component-driven industrial visualization platform inspired by Ignition Perspective. It provides real-time monitoring and control for industrial automation systems.

## Architecture

### Component-Based Design

The system follows a modular, component-based architecture similar to Ignition Perspective:

```
hmi/
├── assets/
│   ├── css/
│   │   └── hmi-common.css        # Shared styles
│   └── js/
│       ├── hmi-common.js          # Core HMI framework
│       └── components.js          # Component library
└── views/
    ├── *.json                     # JSON view definitions (new)
    └── *.html                     # Legacy HTML views
```

### View Definitions (JSON-based)

Views are now defined in JSON format, making them:
- **Declarative**: Define what to display, not how
- **Reusable**: Use shared components across views
- **Maintainable**: Separate data from presentation
- **Configurable**: Easy to modify without code changes

Example JSON view:
```json
{
  "name": "process-overview",
  "title": "Process Overview",
  "header": {
    "title": "Process Overview",
    "icon": "fas fa-industry",
    "actions": [...]
  },
  "components": [
    {
      "type": "metric",
      "label": "Temperature",
      "binding": "process.temperature",
      "unit": "°C"
    }
  ]
}
```

### Available Components

#### Layout Components
- **grid**: Responsive grid layout
- **panel**: Container with header
- **html**: Custom HTML content

#### Display Components
- **metric**: Single value display
- **gauge**: Progress bar with min/max
- **status**: Status indicator with badge
- **table**: Data table with custom renderers
- **chart**: Chart.js integration
- **tank**: SVG tank visualization
- **alarmList**: Alarm display

#### Control Components
- **button**: Action button
- **slider**: Range slider
- **toggle**: Toggle switch

### Data Binding

Components can bind to the HMI data store:

```json
{
  "type": "metric",
  "label": "Temperature",
  "binding": "process.temperature"
}
```

The framework automatically:
- Fetches data from `HMI.getData('process.temperature')`
- Updates UI when data changes
- Handles data formatting and units

### Component Registration

New components can be registered in `components.js`:

```javascript
HMI.registerComponent('myComponent', (config) => {
    return `<div>${config.label}: ${config.value}</div>`;
});
```

## Creating New Views

### 1. JSON View (Recommended)

Create `hmi/views/my-view.json`:

```json
{
  "name": "my-view",
  "title": "My Custom View",
  "header": {
    "title": "My Custom View",
    "icon": "fas fa-chart-bar",
    "actions": [
      {
        "label": "Refresh",
        "icon": "fas fa-sync",
        "action": "refreshView",
        "style": "secondary"
      }
    ]
  },
  "components": [
    {
      "type": "grid",
      "columns": 3,
      "items": [
        { "type": "metric", "label": "Value 1", "binding": "data.value1" },
        { "type": "metric", "label": "Value 2", "binding": "data.value2" },
        { "type": "metric", "label": "Value 3", "binding": "data.value3" }
      ]
    }
  ],
  "onInit": "console.log('View initialized');"
}
```

### 2. Legacy HTML View

Create `hmi/views/my-view.html` with full HTML/CSS/JS. The loader will automatically fall back to HTML if no JSON view is found.

## Data Management

### Setting Data

```javascript
HMI.setData('process.temperature', 75.5);
HMI.setData('equipment.pump1.status', 'running');
```

### Getting Data

```javascript
const temp = HMI.getData('process.temperature');
```

### Data Binding Updates

```javascript
HMI.updateBinding('process.temperature', 76.2);
// Automatically updates all elements with data-bind="process.temperature"
```

## Actions

Define actions in components:

```json
{
  "type": "button",
  "label": "Start Pump",
  "action": "startPump",
  "params": { "pumpId": "P-101" }
}
```

Handle in `hmi-common.js`:

```javascript
HMI.executeAction = function(action, params) {
    switch (action) {
        case 'startPump':
            console.log('Starting pump:', params.pumpId);
            // Call ISA-OS syscall or API
            break;
    }
}
```

## Styling

### Using CSS Variables

The common stylesheet defines CSS variables for consistent styling:

```css
var(--bg-primary)         /* Main background */
var(--bg-secondary)       /* Card background */
var(--text-primary)       /* Primary text */
var(--status-running)     /* Green status */
var(--status-warning)     /* Yellow/orange */
var(--status-fault)       /* Red status */
var(--level-l0)           /* ISA-95 L0 color */
```

### Component Classes

```css
.hmi-card                 /* Standard card */
.metric-card              /* Metric display */
.hmi-table                /* Data table */
.btn-primary              /* Primary button */
.badge-success            /* Success badge */
```

## Current Views

### JSON-based Views
- **process-overview**: Process monitoring with tanks and metrics
- **system-health**: System diagnostics and agent status

### Legacy HTML Views
- **control-panel**: Equipment controls
- **alarms**: ISA-18.2 alarm management
- **trends**: Real-time charting
- **equipment-status**: Equipment monitoring
- **batch-status**: ISA-88 batch execution
- **network-topology**: Network visualization

## Integration with ISA-OS

### Future Integration Points

1. **Syscall Bindings**: Connect components to ISA-OS syscalls
2. **Agent Data**: Display real agent states from kernel
3. **Real-time Updates**: WebSocket connection to OS
4. **Control Actions**: Execute OS commands from HMI

Example integration:
```javascript
// In onInit
HMI.setData('process.temperature', isaOS.syscall.read('L0', 'T-101'));

// In action handler
HMI.executeAction = function(action, params) {
    if (action === 'startPump') {
        isaOS.syscall.write('L0', 'P-101', { command: 'start' });
    }
}
```

## Browser Compatibility

- Chrome/Edge 90+
- Firefox 88+
- Safari 14+

## Performance

- Minimal DOM manipulation
- Efficient data binding
- Chart.js for high-performance graphing
- CSS animations for smooth UI

## License

Part of ISA-OS - Industrial Automation Operating System
