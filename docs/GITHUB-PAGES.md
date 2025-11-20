# GitHub Pages Setup Guide

## Overview

ISA-OS uses GitHub Pages to provide an interactive documentation and sandbox experience. All HTML pages are designed to route through the root `index.html` as the primary entry point.

## Structure

```
isa-os/
├── index.html                          # Main entry point (GitHub Pages root)
├── docs/
│   ├── pages/
│   │   ├── terminal.html              # Full interactive terminal
│   │   └── docs-viewer.html           # Documentation viewer
│   ├── rfcs/                          # RFC specifications
│   │   ├── RFC-001-ISA-HAL.md
│   │   ├── RFC-002-ISA-RT.md
│   │   └── RFC-003-ISA-SEC.md
│   ├── PHASE1-SUMMARY.md              # Phase 1 implementation summary
│   └── GITHUB-PAGES.md                # This file
├── README.md                          # Project README
├── QUICKSTART.md                      # Quick start guide
├── ARCHITECTURE.md                    # Architecture docs
├── MODULES.md                         # Module system docs
├── CHANGELOG.md                       # Version history
├── DOCKER.md                          # Docker deployment
└── INDEX.md                           # File index
```

## Pages

### 1. index.html (Root Entry Point)

**URL:** `https://teslasolar.github.io/isa-os/`

The main landing page featuring:
- Hero section with project overview
- Key features showcase
- ISA-95 architecture diagram
- Interactive sandbox terminal (embedded)
- Documentation links
- Navigation to all other pages

**Key Features:**
- Responsive design
- Embedded terminal sandbox
- Direct links to all documentation
- GitHub Pages header with routing notice
- Font Awesome icons
- Mobile-friendly navigation

### 2. terminal.html (Full Terminal)

**URL:** `https://teslasolar.github.io/isa-os/docs/pages/terminal.html`

Full-screen interactive terminal featuring:
- All ISA-OS commands (help, modules, agents, stats, etc.)
- Command history (up/down arrows)
- Keyboard shortcuts (F1 for help, Ctrl+L to clear)
- Help panel with comprehensive command reference
- Syntax highlighting for output
- Realistic terminal experience

**Commands Available:**
```
System:     help, about, version, stats, uptime, clear
Modules:    modules, module info <name>
Agents:     agents, agent info <id>, agent suspend/resume
Watchdog:   watchdog list/create/enable/disable
L0-L4:      l0 read, l1 write, l2 alarm, l3 batch, l4 erp sync
Config:     config, config get/set
Errors:     errors, errors last, errors clear
```

### 3. docs-viewer.html (Documentation Viewer)

**URL:** `https://teslasolar.github.io/isa-os/docs/pages/docs-viewer.html`

Documentation browser with:
- Sidebar navigation by category
- Markdown rendering (using marked.js)
- GitHub-style markdown CSS
- Responsive sidebar (mobile-friendly)
- Direct links to all docs:
  - Getting Started (README, QUICKSTART)
  - Architecture (ARCHITECTURE, MODULES)
  - RFCs (ISA-HAL, ISA-RT, ISA-SEC)
  - Development (CHANGELOG, PHASE1-SUMMARY, INDEX)
  - Deployment (DOCKER)

## Routing Pattern

All HTML pages include a **GitHub Pages header** that:
1. Identifies the page as part of GitHub Pages
2. Provides a link back to the root `index.html`
3. Ensures proper navigation hierarchy
4. Links to the GitHub repository

**Example Header:**
```html
<div class="gh-header">
    <div>
        <i class="fas fa-home"></i>
        <a href="../../index.html">← Back to ISA-OS Home</a>
    </div>
    <div>
        <i class="fas fa-info-circle"></i>
        GitHub Pages | Route through
        <a href="../../index.html">root index.html</a>
    </div>
    <div>
        <a href="https://github.com/teslasolar/isa-os" target="_blank">
            <i class="fab fa-github"></i> View on GitHub
        </a>
    </div>
</div>
```

## Setup Instructions

### 1. Enable GitHub Pages

1. Go to repository Settings
2. Navigate to **Pages** section
3. Source: **Deploy from a branch**
4. Branch: `main` or `claude/analyze-isa-os-*`
5. Folder: **/ (root)**
6. Save

### 2. Access Your Site

After enabling, site will be available at:
```
https://<username>.github.io/<repository>/
```

For ISA-OS:
```
https://teslasolar.github.io/isa-os/
```

### 3. Custom Domain (Optional)

To use a custom domain:

1. Add `CNAME` file to root:
   ```
   isa-os.yourdomain.com
   ```

2. Configure DNS records:
   ```
   Type: CNAME
   Name: isa-os
   Value: teslasolar.github.io
   ```

3. Update repository settings with custom domain

## Features

### Responsive Design

All pages are mobile-friendly:
- Collapsible navigation on small screens
- Touch-friendly controls
- Responsive grid layouts
- Mobile menu toggle

### Interactive Elements

1. **Terminal Sandbox:**
   - Real-time command execution
   - Syntax highlighting
   - Command history
   - Keyboard shortcuts

2. **Documentation Viewer:**
   - Markdown rendering
   - Category navigation
   - Search-friendly structure
   - GitHub-style formatting

3. **Hero Section:**
   - Animated badges
   - Call-to-action buttons
   - Architecture diagram
   - Feature showcase

### Accessibility

- Semantic HTML5
- ARIA labels where appropriate
- Keyboard navigation support
- Color contrast compliance
- Screen reader friendly

## Customization

### Changing Colors

Edit CSS variables in `<style>` section:

```css
:root {
    --primary: #2563eb;      /* Primary color */
    --secondary: #1e40af;    /* Secondary color */
    --accent: #3b82f6;       /* Accent color */
    --dark: #1e293b;         /* Dark background */
    --success: #10b981;      /* Success color */
    --warning: #f59e0b;      /* Warning color */
    --danger: #ef4444;       /* Danger color */
}
```

### Adding New Pages

1. Create HTML file with GitHub Pages header
2. Add navigation links from `index.html`
3. Update `docs-viewer.html` sidebar if it's documentation
4. Ensure proper routing back to root

### Updating Terminal Commands

Edit `commands` object in `terminal.html`:

```javascript
const commands = {
    mycommand: () => `Output for my command`,
    // ... more commands
};
```

## Dependencies

All dependencies are loaded via CDN (no build required):

- **Font Awesome 6.4.0:** Icons
- **Marked.js:** Markdown parsing (docs-viewer only)
- **GitHub Markdown CSS:** Styling for rendered markdown

## Performance

- Minimal JavaScript
- CDN-hosted dependencies
- No build step required
- Fast load times
- Cached static assets

## SEO

Each page includes:
- Meta description
- Meta keywords
- Semantic HTML structure
- Proper heading hierarchy
- Alt text for icons

## Browser Support

Tested and working on:
- Chrome/Edge (latest)
- Firefox (latest)
- Safari (latest)
- Mobile browsers (iOS Safari, Chrome Android)

## Troubleshooting

### Pages Not Loading

1. Check GitHub Pages is enabled
2. Verify branch and folder settings
3. Wait 1-2 minutes for deployment
4. Check for build errors in Actions tab
5. Ensure file paths are correct

### Relative Links Broken

- Use relative paths from page location
- From `docs/pages/terminal.html` to root: `../../index.html`
- From `docs/pages/terminal.html` to docs: `../PHASE1-SUMMARY.md`
- From `index.html` to pages: `docs/pages/terminal.html`

### Terminal Commands Not Working

1. Check JavaScript console for errors
2. Verify `commands` object is defined
3. Ensure input event listener is attached
4. Test in different browser

## Future Enhancements

Potential additions:
- [ ] Search functionality across documentation
- [ ] Dark mode toggle
- [ ] Syntax highlighting for code blocks
- [ ] Copy-to-clipboard for code snippets
- [ ] Progressive Web App (PWA) support
- [ ] Offline documentation access
- [ ] Interactive architecture diagrams
- [ ] Video tutorials embedded

## Maintenance

### Regular Updates

1. Keep dependencies up to date
2. Test on new browser versions
3. Update documentation links as files change
4. Add new pages for new features
5. Update terminal commands for new syscalls

### Monitoring

Check these regularly:
- GitHub Pages build status
- Broken links
- Console errors
- Mobile responsiveness
- Load times

## Resources

- **GitHub Pages Docs:** https://docs.github.com/en/pages
- **Marked.js:** https://marked.js.org/
- **Font Awesome:** https://fontawesome.com/
- **GitHub Markdown CSS:** https://github.com/sindresorhus/github-markdown-css

---

**Questions or Issues?**

Open an issue on GitHub: https://github.com/teslasolar/isa-os/issues
