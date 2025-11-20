/**
 * ISA-OS HMI Common Utilities
 * Perspective-style component framework
 */

const HMI = {
    // Data store for view state
    dataStore: {},

    // Active views
    activeViews: new Map(),

    // Component registry
    components: {},

    /**
     * Initialize HMI system
     */
    init() {
        console.log('[HMI] Initializing ISA-OS HMI System');
        this.startDataRefresh();
    },

    /**
     * Load a view from JSON definition
     */
    async loadView(viewName) {
        try {
            const response = await fetch(`hmi/views/${viewName}.json`);
            const viewDef = await response.json();

            console.log(`[HMI] Loading view: ${viewName}`);

            // Render the view
            const html = this.renderView(viewDef);

            // Update DOM
            const container = document.getElementById('view-content');
            container.innerHTML = html;

            // Initialize view
            if (viewDef.onInit) {
                const initFn = new Function('HMI', viewDef.onInit);
                initFn(this);
            }

            // Store active view
            this.activeViews.set('current', { name: viewName, definition: viewDef });

            return viewDef;
        } catch (error) {
            console.error(`[HMI] Failed to load view ${viewName}:`, error);
            throw error;
        }
    },

    /**
     * Render view from JSON definition
     */
    renderView(viewDef) {
        let html = '';

        // Render header
        if (viewDef.header) {
            html += this.renderComponent('header', viewDef.header);
        }

        // Render components
        if (viewDef.components) {
            for (const component of viewDef.components) {
                html += this.renderComponent(component.type, component);
            }
        }

        return html;
    },

    /**
     * Render a component
     */
    renderComponent(type, config) {
        const renderer = this.components[type];
        if (!renderer) {
            console.warn(`[HMI] Unknown component type: ${type}`);
            return '';
        }

        return renderer(config, this);
    },

    /**
     * Register a component renderer
     */
    registerComponent(type, renderer) {
        this.components[type] = renderer;
        console.log(`[HMI] Registered component: ${type}`);
    },

    /**
     * Update data binding
     */
    updateBinding(path, value) {
        this.setData(path, value);

        // Update all bound elements
        document.querySelectorAll(`[data-bind="${path}"]`).forEach(el => {
            if (el.tagName === 'INPUT') {
                el.value = value;
            } else {
                el.textContent = value;
            }
        });
    },

    /**
     * Get data from store
     */
    getData(path) {
        return path.split('.').reduce((obj, key) => obj?.[key], this.dataStore);
    },

    /**
     * Set data in store
     */
    setData(path, value) {
        const keys = path.split('.');
        const lastKey = keys.pop();
        const target = keys.reduce((obj, key) => {
            if (!obj[key]) obj[key] = {};
            return obj[key];
        }, this.dataStore);
        target[lastKey] = value;
    },

    /**
     * Format value with units
     */
    formatValue(value, unit = '') {
        if (typeof value === 'number') {
            return value.toFixed(1) + (unit ? ' ' + unit : '');
        }
        return value + (unit ? ' ' + unit : '');
    },

    /**
     * Generate random process value with realistic variation
     */
    generateProcessValue(base, variance, min, max) {
        let value = base + (Math.random() - 0.5) * variance * 2;

        // Occasional spikes
        if (Math.random() < 0.05) {
            value += (Math.random() - 0.5) * variance * 3;
        }

        return Math.max(min, Math.min(max, value));
    },

    /**
     * Start automatic data refresh
     */
    startDataRefresh() {
        setInterval(() => {
            this.refreshSimulatedData();
        }, 2000);
    },

    /**
     * Refresh simulated process data
     */
    refreshSimulatedData() {
        // Simulate process values
        this.setData('process.temperature', this.generateProcessValue(75, 5, 60, 90));
        this.setData('process.pressure', this.generateProcessValue(3.5, 0.5, 2, 5));
        this.setData('process.flow', this.generateProcessValue(120, 20, 80, 160));
        this.setData('process.level', this.generateProcessValue(65, 10, 40, 80));

        // System metrics
        this.setData('system.cpu', this.generateProcessValue(23, 5, 10, 80));
        this.setData('system.memory', this.generateProcessValue(512, 32, 400, 800));
        this.setData('system.uptime', Date.now() - (72 * 3600 * 1000));

        // Trigger custom refresh events
        document.dispatchEvent(new CustomEvent('hmi:dataRefresh'));
    },

    /**
     * Create HTML element from config
     */
    createElement(tag, attrs = {}, children = []) {
        let html = `<${tag}`;

        for (const [key, value] of Object.entries(attrs)) {
            html += ` ${key}="${value}"`;
        }

        html += '>';

        if (typeof children === 'string') {
            html += children;
        } else if (Array.isArray(children)) {
            html += children.join('');
        }

        html += `</${tag}>`;

        return html;
    },

    /**
     * Show notification
     */
    notify(message, type = 'info') {
        console.log(`[HMI] ${type.toUpperCase()}: ${message}`);
        // Could implement toast notifications here
    },

    /**
     * Execute action
     */
    executeAction(action, params = {}) {
        console.log(`[HMI] Executing action: ${action}`, params);

        // Common actions
        switch (action) {
            case 'startPump':
                this.notify('Pump started', 'success');
                break;
            case 'stopPump':
                this.notify('Pump stopped', 'warning');
                break;
            case 'acknowledgeAlarm':
                this.notify('Alarm acknowledged', 'info');
                break;
            case 'exportData':
                this.exportToCSV(params.data, params.filename);
                break;
            default:
                console.warn(`[HMI] Unknown action: ${action}`);
        }
    },

    /**
     * Export data to CSV
     */
    exportToCSV(data, filename) {
        const csv = Array.isArray(data)
            ? data.map(row => Object.values(row).join(',')).join('\n')
            : data;

        const blob = new Blob([csv], { type: 'text/csv' });
        const url = URL.createObjectURL(blob);
        const a = document.createElement('a');
        a.href = url;
        a.download = filename || `export_${Date.now()}.csv`;
        a.click();
        URL.revokeObjectURL(url);
    },

    /**
     * Get status color
     */
    getStatusColor(value, thresholds) {
        if (!thresholds) return 'var(--status-info)';

        if (value >= thresholds.critical) return 'var(--status-fault)';
        if (value >= thresholds.high) return 'var(--status-warning)';
        if (value >= thresholds.medium) return 'var(--status-info)';
        return 'var(--status-running)';
    },

    /**
     * Format timestamp
     */
    formatTimestamp(timestamp) {
        if (!timestamp) return '--:--:--';
        const date = new Date(timestamp);
        return date.toLocaleTimeString();
    },

    /**
     * Format duration
     */
    formatDuration(ms) {
        const hours = Math.floor(ms / 3600000);
        const minutes = Math.floor((ms % 3600000) / 60000);
        const seconds = Math.floor((ms % 60000) / 1000);

        if (hours > 0) {
            return `${hours}h ${minutes}m`;
        } else if (minutes > 0) {
            return `${minutes}m ${seconds}s`;
        } else {
            return `${seconds}s`;
        }
    }
};

// Initialize on load
if (typeof window !== 'undefined') {
    window.HMI = HMI;
    window.addEventListener('DOMContentLoaded', () => HMI.init());
}
