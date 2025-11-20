# How to Access the ISA-OS HMI System

## Option 1: From GitHub Pages (Deployed)

If viewing on GitHub Pages, navigate to:
```
https://[your-username].github.io/isa-os/docs/pages/hmi.html
```

Or from the main page:
```
https://[your-username].github.io/isa-os/
```
Then click **"Launch HMI"** button in the hero section.

## Option 2: From Local Files

If running locally, open:
```
file:///path/to/isa-os/docs/pages/hmi.html
```

Or start from:
```
file:///path/to/isa-os/index.html
```
Then click **"Launch HMI"** button.

## Option 3: Using Python HTTP Server

```bash
cd /path/to/isa-os
python3 -m http.server 8000
```

Then open browser to:
```
http://localhost:8000/docs/pages/hmi.html
```

## Navigation Within HMI

Once in the HMI, use the **left sidebar** to navigate between views:

### Process Views (JSON-based - New Architecture)
- ✅ **Process Overview** - Tanks, metrics, equipment
- ✅ **System Health** - CPU, memory, agents, watchdogs

### Legacy Views (HTML-based)
- Control Panel
- Equipment Status
- Alarms (ISA-18.2)
- Trends
- Batch Status (ISA-88)
- Network Topology

## Troubleshooting

### "Can't see HMI button on main page"
- Clear browser cache (Ctrl+Shift+R or Cmd+Shift+R)
- Wait for GitHub Pages to deploy (check Actions tab on GitHub)
- Make sure you're on the latest commit: `10ed8f3`

### "HMI page loads but views don't work"
- Check browser console for errors (F12)
- Verify all assets are loading:
  - `hmi/assets/css/hmi-common.css`
  - `hmi/assets/js/hmi-common.js`
  - `hmi/assets/js/components.js`

### "404 Not Found"
- Ensure the path is correct relative to where you opened the page
- If using GitHub Pages, wait for deployment to complete

## File Structure

```
isa-os/
├── index.html                          ← Main landing page
└── docs/
    └── pages/
        ├── hmi.html                    ← HMI main loader
        ├── terminal.html               ← Terminal
        └── hmi/
            ├── README.md               ← HMI architecture docs
            ├── assets/
            │   ├── css/
            │   │   └── hmi-common.css  ← Shared styles
            │   └── js/
            │       ├── hmi-common.js   ← Framework
            │       └── components.js   ← Components
            └── views/
                ├── *.json              ← New JSON views
                └── *.html              ← Legacy HTML views
```

## Quick Links

From `index.html`:
1. **Hero section**: "Launch HMI" button (primary CTA)
2. **Sandbox section**: "HMI Dashboard" button (green)
3. **Documentation grid**: "HMI Dashboard" card (highlighted)

All point to: `docs/pages/hmi.html`
