/**
 * ISA-OS HMI Component Library
 * Reusable Perspective-style components
 */

// Header Component
HMI.registerComponent('header', (config) => {
    return `
        <div class="view-header">
            <h1 class="view-title">
                ${config.icon ? `<i class="${config.icon}"></i>` : ''}
                ${config.title}
            </h1>
            <div class="view-actions">
                ${(config.actions || []).map(action => `
                    <button class="btn btn-${action.style || 'secondary'}"
                            onclick="HMI.executeAction('${action.action}', ${JSON.stringify(action.params || {})})">
                        ${action.icon ? `<i class="${action.icon}"></i>` : ''}
                        ${action.label}
                    </button>
                `).join('')}
            </div>
        </div>
    `;
});

// Grid Container
HMI.registerComponent('grid', (config) => {
    const columns = config.columns || 'auto-fit';
    const gridClass = columns === 'auto-fit' ? 'grid-auto-fit' : `grid-${columns}`;

    return `
        <div class="${gridClass}" style="${config.style || ''}">
            ${(config.items || []).map(item => HMI.renderComponent(item.type, item)).join('')}
        </div>
    `;
});

// Metric Card Component
HMI.registerComponent('metric', (config) => {
    const value = config.binding ? HMI.getData(config.binding) : config.value;

    return `
        <div class="hmi-card ${config.className || ''}">
            <div class="metric-card">
                <div class="metric-label">${config.label}</div>
                <div class="metric-value" ${config.binding ? `data-bind="${config.binding}"` : ''}>
                    ${HMI.formatValue(value, config.unit)}
                </div>
            </div>
        </div>
    `;
});

// Status Indicator Component
HMI.registerComponent('status', (config) => {
    const status = config.binding ? HMI.getData(config.binding) : config.status;

    return `
        <div class="hmi-card ${config.className || ''}">
            <div class="flex items-center justify-between">
                <div>
                    <div class="text-secondary" style="font-size: 0.875rem; margin-bottom: 0.5rem;">
                        ${config.label}
                    </div>
                    <div class="badge badge-${status || 'secondary'}">
                        ${(status || 'unknown').toUpperCase()}
                    </div>
                </div>
                <span class="status-indicator ${status || 'stopped'}"></span>
            </div>
        </div>
    `;
});

// Gauge Component
HMI.registerComponent('gauge', (config) => {
    const value = config.binding ? HMI.getData(config.binding) || 0 : config.value || 0;
    const min = config.min || 0;
    const max = config.max || 100;
    const percentage = ((value - min) / (max - min)) * 100;

    let fillClass = 'health-fill excellent';
    if (percentage > 75) fillClass = 'health-fill poor';
    else if (percentage > 50) fillClass = 'health-fill fair';
    else if (percentage > 25) fillClass = 'health-fill good';

    return `
        <div class="hmi-card ${config.className || ''}">
            <div class="health-bar">
                <div class="health-label">
                    <span>${config.label}</span>
                    <strong ${config.binding ? `data-bind="${config.binding}"` : ''}>
                        ${HMI.formatValue(value, config.unit)}
                    </strong>
                </div>
                <div class="health-progress">
                    <div class="${fillClass}" style="width: ${percentage}%"></div>
                </div>
            </div>
        </div>
    `;
});

// Table Component
HMI.registerComponent('table', (config) => {
    const data = config.binding ? HMI.getData(config.binding) || [] : config.data || [];

    return `
        <div class="hmi-card ${config.className || ''}">
            ${config.title ? `<h3 class="mb-2">${config.title}</h3>` : ''}
            <div style="overflow-x: auto;">
                <table class="hmi-table">
                    <thead>
                        <tr>
                            ${config.columns.map(col => `<th>${col.label}</th>`).join('')}
                        </tr>
                    </thead>
                    <tbody>
                        ${data.map(row => `
                            <tr>
                                ${config.columns.map(col => {
                                    let value = row[col.field];
                                    if (col.render) {
                                        const renderFn = new Function('value', 'row', col.render);
                                        value = renderFn(value, row);
                                    }
                                    return `<td>${value}</td>`;
                                }).join('')}
                            </tr>
                        `).join('')}
                    </tbody>
                </table>
            </div>
        </div>
    `;
});

// Chart Component
HMI.registerComponent('chart', (config) => {
    const chartId = `chart-${Math.random().toString(36).substr(2, 9)}`;

    // Store chart config for initialization
    setTimeout(() => {
        const ctx = document.getElementById(chartId);
        if (!ctx) return;

        const data = config.binding ? HMI.getData(config.binding) || [] : config.data || [];

        new Chart(ctx, {
            type: config.chartType || 'line',
            data: {
                labels: data.map(d => d.x || d.label),
                datasets: [{
                    label: config.label,
                    data: data.map(d => d.y || d.value),
                    borderColor: config.color || '#3b82f6',
                    backgroundColor: config.fill ? (config.color || '#3b82f6') + '33' : undefined,
                    fill: config.fill || false,
                    tension: 0.4
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                plugins: {
                    legend: { display: config.showLegend !== false }
                },
                scales: {
                    y: {
                        min: config.min,
                        max: config.max
                    }
                }
            }
        });
    }, 100);

    return `
        <div class="hmi-card ${config.className || ''}">
            ${config.title ? `<h3 class="mb-2">${config.title}</h3>` : ''}
            <div style="height: ${config.height || '300px'};">
                <canvas id="${chartId}"></canvas>
            </div>
        </div>
    `;
});

// Button Component
HMI.registerComponent('button', (config) => {
    return `
        <button class="btn btn-${config.style || 'primary'} ${config.className || ''}"
                onclick="HMI.executeAction('${config.action}', ${JSON.stringify(config.params || {})})">
            ${config.icon ? `<i class="${config.icon}"></i>` : ''}
            ${config.label}
        </button>
    `;
});

// Slider Control Component
HMI.registerComponent('slider', (config) => {
    const value = config.binding ? HMI.getData(config.binding) || 0 : config.value || 0;
    const sliderId = `slider-${Math.random().toString(36).substr(2, 9)}`;

    return `
        <div class="hmi-card ${config.className || ''}">
            <div class="form-group">
                <label class="form-label">
                    ${config.label}
                    <span class="float-right" id="${sliderId}-value">${value} ${config.unit || ''}</span>
                </label>
                <input type="range"
                       class="slider"
                       id="${sliderId}"
                       min="${config.min || 0}"
                       max="${config.max || 100}"
                       value="${value}"
                       ${config.binding ? `data-bind="${config.binding}"` : ''}
                       oninput="document.getElementById('${sliderId}-value').textContent = this.value + ' ${config.unit || ''}'; ${config.onChange || ''}">
            </div>
        </div>
    `;
});

// Toggle Switch Component
HMI.registerComponent('toggle', (config) => {
    const checked = config.binding ? HMI.getData(config.binding) : config.checked;
    const toggleId = `toggle-${Math.random().toString(36).substr(2, 9)}`;

    return `
        <div class="hmi-card ${config.className || ''}">
            <div class="flex items-center justify-between">
                <label class="form-label">${config.label}</label>
                <label class="toggle-switch">
                    <input type="checkbox"
                           id="${toggleId}"
                           ${checked ? 'checked' : ''}
                           ${config.binding ? `data-bind="${config.binding}"` : ''}
                           onchange="${config.onChange || ''}">
                    <span class="toggle-slider"></span>
                </label>
            </div>
        </div>
    `;
});

// SVG Tank Component
HMI.registerComponent('tank', (config) => {
    const level = config.binding ? HMI.getData(config.binding) || 0 : config.level || 0;
    const tankId = `tank-${Math.random().toString(36).substr(2, 9)}`;

    return `
        <div class="hmi-card ${config.className || ''}" style="text-align: center;">
            <svg width="${config.width || 120}" height="${config.height || 200}" viewBox="0 0 120 200">
                <defs>
                    <linearGradient id="${tankId}-gradient" x1="0%" y1="0%" x2="0%" y2="100%">
                        <stop offset="0%" style="stop-color:${config.color || '#3b82f6'};stop-opacity:0.8" />
                        <stop offset="100%" style="stop-color:${config.color || '#3b82f6'};stop-opacity:0.4" />
                    </linearGradient>
                </defs>

                <!-- Tank outline -->
                <rect x="30" y="20" width="60" height="160" fill="none" stroke="#64748b" stroke-width="2"/>

                <!-- Liquid -->
                <rect x="32" y="${180 - (160 * level / 100)}" width="56" height="${160 * level / 100}"
                      fill="url(#${tankId}-gradient)" />

                <!-- Level text -->
                <text x="60" y="195" text-anchor="middle" fill="#cbd5e1" font-size="14" font-weight="bold">
                    ${level.toFixed(0)}%
                </text>
            </svg>
            <div class="text-center mt-1" style="font-size: 0.875rem; color: var(--text-secondary);">
                ${config.label}
            </div>
        </div>
    `;
});

// Alarm List Component
HMI.registerComponent('alarmList', (config) => {
    const alarms = config.binding ? HMI.getData(config.binding) || [] : config.alarms || [];

    return `
        <div class="hmi-card ${config.className || ''}">
            <h3 class="mb-2">${config.title || 'Alarms'}</h3>
            <div style="max-height: ${config.maxHeight || '400px'}; overflow-y: auto;">
                ${alarms.length === 0 ? '<p class="text-center text-secondary">No active alarms</p>' : ''}
                ${alarms.map(alarm => `
                    <div class="hmi-card mb-1" style="padding: 0.75rem; border-left: 4px solid var(--priority-${alarm.priority});">
                        <div class="flex justify-between items-center">
                            <div>
                                <div style="font-weight: 600;">${alarm.message}</div>
                                <div style="font-size: 0.75rem; color: var(--text-secondary);">
                                    ${alarm.source} | ${HMI.formatTimestamp(alarm.timestamp)}
                                </div>
                            </div>
                            <span class="badge badge-${alarm.priority}">${alarm.priority}</span>
                        </div>
                    </div>
                `).join('')}
            </div>
        </div>
    `;
});

// Custom HTML Component
HMI.registerComponent('html', (config) => {
    return config.content || '';
});

// Section/Panel Component
HMI.registerComponent('panel', (config) => {
    return `
        <div class="hmi-card ${config.className || ''}">
            ${config.title ? `<h3 class="mb-2">
                ${config.icon ? `<i class="${config.icon}"></i>` : ''}
                ${config.title}
            </h3>` : ''}
            ${(config.components || []).map(comp => HMI.renderComponent(comp.type, comp)).join('')}
        </div>
    `;
});

console.log('[HMI] Component library loaded');
