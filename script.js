// Global variable for the Chart
let sensorChart;
let chartData = {
  labels: [],
  proximity: [],
  temperature: [],
  humidity: [],
  battery: [],
  solarOutput: []
};
const MAX_DATA_POINTS = 20;

// ========================================================================
// === 1. NEW DATA FETCHING (This replaces all simulation functions) ===
// ========================================================================

/**
 * Main data fetching function.
 * This runs on an interval and updates the entire dashboard.
 */
async function fetchDataFromESP32() {
  const ip = localStorage.getItem('esp32IP');
  if (!ip) {
    console.warn('ESP32 IP not set. Skipping data fetch.');
    // Optionally update UI to show "IP Not Set"
    document.getElementById('connectionText').textContent = 'IP Not Set';
    const connectionDot = document.querySelector('.connection-dot');
    connectionDot.style.backgroundColor = 'var(--danger)';
    connectionDot.classList.remove('connected');
    return;
  }

  const url = `http://${ip}/api/sensors`;

  try {
    const response = await fetch(url, { signal: AbortSignal.timeout(4000) }); // 4-second timeout
    if (!response.ok) {
      throw new Error(`HTTP error! status: ${response.status}`);
    }
    const data = await response.json();

    // --- We got data! Update the dashboard ---
    updateStatusCards(data);
    updateLogs(data.logs);
    updateRtcAndPower(data);
    updateChart(data);

    // Update connection status to "Connected"
    const connectionDot = document.querySelector('.connection-dot');
    document.getElementById('connectionText').textContent = 'Device is Connected';
    connectionDot.style.backgroundColor = 'var(--secondary)';
    connectionDot.classList.add('connected');

  } catch (error) {
    console.error('Fetch error:', error);
    // Update connection status to "Connection Lost"
    const connectionDot = document.querySelector('.connection-dot');
    document.getElementById('connectionText').textContent = 'Connection Lost';
    connectionDot.style.backgroundColor = 'var(--danger)';
    connectionDot.classList.remove('connected');
  }
}

/**
 * Updates the Capacitive and Temperature sensor cards
 */
function updateStatusCards(data) {
  // 1. Capacitive Proximity Sensor
  const proximityValue = document.getElementById('proxValue');
  const proximityStatus = document.getElementById('proxStatus');
  const proximityUpdate = document.getElementById('proxUpdate');
  
  // Use data from ESP32
  proximityValue.textContent = data.fence.status; // e.g., "ACTIVE" or "CLEAR"
  proximityUpdate.textContent = "Just now";
  
  if (data.fence.status === "ACTIVE") {
    proximityStatus.textContent = "Current Detected!";
    proximityStatus.className = "status-badge status-alert";
  } else {
    proximityStatus.textContent = "No Current";
    proximityStatus.className = "status-badge status-normal";
  }
  
  // 2. Temperature Sensor
  const temperatureValue = document.getElementById('tempValue');
  const temperatureStatus = document.getElementById('tempStatus');
  
  temperatureValue.textContent = data.temperature.value.toFixed(1) + '°C';
  
  if (data.temperature.value > 35) {
    temperatureStatus.textContent = "High Heat Alert";
    temperatureStatus.className = "status-badge status-alert";
  } else if (data.temperature.value > 30) {
    temperatureStatus.textContent = "Elevated";
    temperatureStatus.className = "status-badge status-warning";
  } else {
    temperatureStatus.textContent = "Optimal";
    temperatureStatus.className = "status-badge status-normal";
  }
  
  // 3. Update Humidity on Temp Card
  const humidityValue = document.querySelector('#tempValue + .status-badge + .sensor-details .sensor-detail:nth-child(3) .detail-value');
  if (humidityValue) {
    humidityValue.textContent = data.humidity.toFixed(1) + '%';
  }
}

/**
 * Updates the Detection and RFID log cards
 */
function updateLogs(logs) {
  const detectLog = document.getElementById('detectLog');
  const rfidLog = document.getElementById('rfidLog');

  // 1. Clear existing *simulated* logs
  detectLog.innerHTML = '';
  rfidLog.innerHTML = '';

  // 2. Populate Detection Log
  if (logs && logs.detect) {
    logs.detect.forEach(log => {
      const entry = document.createElement('div');
      entry.className = 'log-entry';
      entry.innerHTML = `<i class="fas ${log.icon} log-icon"></i>
                        <span class="log-time">${log.time}</span>
                        <span class="log-message">${log.msg}</span>`;
      detectLog.prepend(entry);
    });
  }

  // 3. Populate RFID Log
  if (logs && logs.rfid) {
    logs.rfid.forEach(log => {
      const entry = document.createElement('div');
      entry.className = 'log-entry';
      entry.innerHTML = `<i class="fas ${log.icon} log-icon"></i>
                        <span class="log-time">${log.time}</span>
                        <span class="log-message">${log.msg}</span>`;
      rfidLog.prepend(entry);
    });
  }
}

/**
 * Updates the RTC Time and Solar/Battery card
 */
function updateRtcAndPower(data) {
  // 1. RTC Time
  const now = new Date(data.time); // Use time from ESP32's RTC
  document.getElementById('rtcTime').textContent = now.toLocaleTimeString();
  document.getElementById('rtcDate').textContent = now.toLocaleDateString('en-CA');
  
  // 2. Battery
  const batteryLevel = document.getElementById('batteryLevel');
  const batteryText = document.getElementById('batteryText');
  const newWidth = Math.max(0, Math.min(100, data.battery.level));
  
  batteryLevel.style.width = newWidth + '%';
  batteryText.textContent = `Battery: ${Math.round(newWidth)}%`;

  // 3. Solar
  const solarInput = document.querySelector('.battery-container .detail-label');
  const solarPower = document.querySelector('.solar-container .detail-label');
  
  if(solarInput) solarInput.textContent = `Solar Input: ${data.solar.voltage.toFixed(1)}V`;
  if(solarPower) solarPower.textContent = `${data.solar.current.toFixed(1)}A / ${data.solar.power.toFixed(1)}W`;
  
  // 4. Uptime (optional, ESP32 can send this)
  // document.getElementById('uptime').textContent = data.uptime; 
}

/**
 * Updates the live-data chart
 */
function updateChart(data) {
  const now = new Date(data.time);
  const timeLabel = now.toLocaleTimeString();

  // Add new data
  chartData.labels.push(timeLabel);
  chartData.proximity.push(data.fence.status === "ACTIVE" ? 50 : 0); // Show "50cm" for active, "0" for clear
  chartData.temperature.push(data.temperature.value);
  chartData.humidity.push(data.humidity);
  chartData.battery.push(data.battery.level);
  chartData.solarOutput.push(data.solar.power);

  // Keep only the last MAX_DATA_POINTS
  if (chartData.labels.length > MAX_DATA_POINTS) {
    chartData.labels.shift();
    chartData.proximity.shift();
    chartData.temperature.shift();
    chartData.humidity.shift();
    chartData.battery.shift();
    chartData.solarOutput.shift();
  }

  // Update chart
  if (sensorChart) {
    sensorChart.update('none');
  }
}


// ==========================================================
// === 2. UNCHANGED FUNCTIONS (From your original script.js) ===
// ==========================================================

// Update RTC time every second (using *local* time, will be overwritten by ESP32)
function updateTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString();
  const dateString = now.toLocaleDateString('en-CA');
  
  document.getElementById('rtcTime').textContent = timeString;
  document.getElementById('rtcDate').textContent = dateString;
}

// ESP32 IP Address functionality
function initializeESP32Info() {
  const ipInput = document.getElementById('esp32IpInput');
  const editBtn = document.getElementById('editIpBtn');
  const saveBtn = document.getElementById('saveIpBtn');
  const connectionDot = document.querySelector('.connection-dot');
  const connectionText = document.getElementById('connectionText');
  
  // Load saved IP from localStorage
  const savedIP = localStorage.getItem('esp32IP');
  if (savedIP) {
    ipInput.value = savedIP;
    ipInput.disabled = true;
    editBtn.classList.remove('hidden');
    saveBtn.classList.add('hidden');
    connectionText.textContent = 'Connecting...';
    connectionDot.style.backgroundColor = 'var(--warning)';
  } else {
    // No saved IP - ready for user input
    ipInput.disabled = false;
    ipInput.focus();
    editBtn.classList.add('hidden');
    saveBtn.classList.remove('hidden');
    connectionText.textContent = 'Editing...';
    connectionDot.style.backgroundColor = 'var(--warning)';
  }
  
  // Edit button click handler
  editBtn.addEventListener('click', function() {
    ipInput.disabled = false;
    ipInput.focus();
    ipInput.select();
    editBtn.classList.add('hidden');
    saveBtn.classList.remove('hidden');
    connectionText.textContent = 'Editing...';
    connectionDot.style.backgroundColor = 'var(--warning)';
  });
  
  // Save button click handler
  saveBtn.addEventListener('click', function() {
    const newIP = ipInput.value.trim();
    
    // Check if IP is empty
    if (!newIP) {
      alert('Please enter an IP address');
      ipInput.focus();
      return;
    }
    
    // Basic IP validation
    const ipPattern = /^(\d{1,3}\.){3}\d{1,3}$/;
    if (ipPattern.test(newIP)) {
      // Save to localStorage
      localStorage.setItem('esp32IP', newIP);
      
      ipInput.disabled = true;
      editBtn.classList.remove('hidden');
      saveBtn.classList.add('hidden');
      
      // Show connecting feedback
      connectionText.textContent = 'Connecting...';
      connectionDot.style.backgroundColor = 'var(--warning)';
      
      // Trigger an immediate fetch
      fetchDataFromESP32();
    } else {
      alert('Please enter a valid IP address (e.g., 192.168.1.100)');
      ipInput.focus();
    }
  });
  
  // Allow saving with Enter key
  ipInput.addEventListener('keypress', function(e) {
    if (e.key === 'Enter' && !ipInput.disabled) {
      saveBtn.click();
    }
  });
  
  // Cancel edit on Escape key
  ipInput.addEventListener('keydown', function(e) {
    if (e.key === 'Escape' && !ipInput.disabled) {
      const savedIP = localStorage.getItem('esp32IP');
      if (savedIP) {
        ipInput.value = savedIP;
        ipInput.disabled = true;
        editBtn.classList.remove('hidden');
        saveBtn.classList.add('hidden');
        connectionText.textContent = 'Connecting...';
        connectionDot.style.backgroundColor = 'var(--warning)';
      } else {
        ipInput.value = '';
        connectionText.textContent = 'Editing...';
        connectionDot.style.backgroundColor = 'var(--warning)';
      }
    }
  });
}

// Modal functionality (Unchanged)
function initializeModals() {
  const aboutBtn = document.getElementById('aboutBtn');
  const notificationBtn = document.getElementById('notificationBtn');
  const settingsBtn = document.getElementById('settingsBtn');
  const aboutModal = document.getElementById('aboutModal');
  const notificationModal = document.getElementById('notificationModal');
  const settingsModal = document.getElementById('settingsModal');
  const closeAbout = document.getElementById('closeAbout');
  const closeNotification = document.getElementById('closeNotification');
  const closeSettings = document.getElementById('closeSettings');

  // Open about modal
  aboutBtn.addEventListener('click', function() {
    aboutModal.classList.add('show');
  });

  // Open notification modal
  notificationBtn.addEventListener('click', function() {
    notificationModal.classList.add('show');
  });

  // Open settings modal
  settingsBtn.addEventListener('click', function() {
    settingsModal.classList.add('show');
  });

  // Close about modal
  closeAbout.addEventListener('click', function() {
    aboutModal.classList.remove('show');
  });

  // Close notification modal
  closeNotification.addEventListener('click', function() {
    notificationModal.classList.remove('show');
  });

  // Close settings modal
  closeSettings.addEventListener('click', function() {
    settingsModal.classList.remove('show');
  });

  // Close modals when clicking outside
  window.addEventListener('click', function(e) {
    if (e.target === aboutModal) {
      aboutModal.classList.remove('show');
    }
    if (e.target === notificationModal) {
      notificationModal.classList.remove('show');
    }
    if (e.target === settingsModal) {
      settingsModal.classList.remove('show');
    }
  });

  // Close modals with Escape key
  document.addEventListener('keydown', function(e) {
    if (e.key === 'Escape') {
      aboutModal.classList.remove('show');
      notificationModal.classList.remove('show');
      settingsModal.classList.remove('show');
    }
  });

  // Tab functionality for About modal
  const tabBtns = document.querySelectorAll('.tab-btn');
  const tabContents = document.querySelectorAll('.tab-content');

  tabBtns.forEach(btn => {
    btn.addEventListener('click', function() {
      const tabName = this.getAttribute('data-tab');
      
      // Remove active class from all tabs and contents
      tabBtns.forEach(b => b.classList.remove('active'));
      tabContents.forEach(c => c.classList.remove('active'));
      
      // Add active class to clicked tab and corresponding content
      this.classList.add('active');
      document.getElementById(tabName + '-tab').classList.add('active');
    });
  });
}

// Settings functionality (Unchanged)
function initializeSettings() {
  const saveBtn = document.querySelector('.btn-save');
  const resetBtn = document.querySelector('.btn-reset');

  saveBtn.addEventListener('click', function() {
    // Get all settings values
    const settings = {
      refreshRate: document.getElementById('refreshRate').value,
      theme: document.getElementById('theme').value,
      enableNotifications: document.getElementById('enableNotifications').checked,
      soundAlerts: document.getElementById('soundAlerts').checked,
      emailNotifications: document.getElementById('emailNotifications').checked,
      timeFormat: document.getElementById('timeFormat').value,
      tempUnit: document.getElementById('tempUnit').value
    };

    // Save to localStorage
    localStorage.setItem('fenceora_settings', JSON.stringify(settings));

    // Show success feedback
    saveBtn.innerHTML = '<i class="fas fa-check"></i> Saved!';
    saveBtn.style.background = 'linear-gradient(135deg, var(--secondary), var(--secondary))';

    setTimeout(() => {
      saveBtn.innerHTML = '<i class="fas fa-save"></i> Save Settings';
      saveBtn.style.background = 'linear-gradient(135deg, var(--primary), var(--secondary))';
      document.getElementById('settingsModal').classList.remove('show');
    }, 1500);
  });

  resetBtn.addEventListener('click', function() {
    if (confirm('Reset all settings to default?')) {
      localStorage.removeItem('fenceora_settings');
      location.reload();
    }
  });

  // Load saved settings
  const savedSettings = localStorage.getItem('fenceora_settings');
  if (savedSettings) {
    const settings = JSON.parse(savedSettings);
    document.getElementById('refreshRate').value = settings.refreshRate || '5000';
    document.getElementById('theme').value = settings.theme || 'dark';
    document.getElementById('enableNotifications').checked = settings.enableNotifications !== false;
    document.getElementById('soundAlerts').checked = settings.soundAlerts !== false;
    document.getElementById('emailNotifications').checked = settings.emailNotifications || false;
    document.getElementById('timeFormat').value = settings.timeFormat || '24';
    document.getElementById('tempUnit').value = settings.tempUnit || 'C';
  }
}

// Initialize Real-Time Graph (Unchanged)
function initializeChart() {
  const ctx = document.getElementById('sensorChart').getContext('2d');
  
  sensorChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: chartData.labels,
      datasets: [
        {
          label: 'Fence Status (50=Active)',
          data: chartData.proximity,
          borderColor: '#00d9ff',
          backgroundColor: 'rgba(0, 217, 255, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          pointBackgroundColor: '#00d9ff',
          pointBorderColor: '#00d9ff',
          pointHoverRadius: 6,
          yAxisID: 'y-proximity'
        },
        {
          label: 'Temperature (°C)',
          data: chartData.temperature,
          borderColor: '#ff6b6b',
          backgroundColor: 'rgba(255, 107, 107, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          pointBackgroundColor: '#ff6b6b',
          pointBorderColor: '#ff6b6b',
          pointHoverRadius: 6,
          yAxisID: 'y-temp'
        },
        {
          label: 'Humidity (%)',
          data: chartData.humidity,
          borderColor: '#4ecdc4',
          backgroundColor: 'rgba(78, 205, 196, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          pointBackgroundColor: '#4ecdc4',
          pointBorderColor: '#4ecdc4',
          pointHoverRadius: 6,
          yAxisID: 'y-percentage'
        },
        {
          label: 'Battery (%)',
          data: chartData.battery,
          borderColor: '#00ffae',
          backgroundColor: 'rgba(0, 255, 174, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          pointBackgroundColor: '#00ffae',
          pointBorderColor: '#00ffae',
          pointHoverRadius: 6,
          yAxisID: 'y-percentage'
        },
        {
          label: 'Solar Panel Output (W)',
          data: chartData.solarOutput,
          borderColor: '#ffd700',
          backgroundColor: 'rgba(255, 215, 0, 0.1)',
          borderWidth: 2,
          tension: 0.4,
          fill: true,
          pointRadius: 4,
          pointBackgroundColor: '#ffd700',
          pointBorderColor: '#ffd700',
          pointHoverRadius: 6,
          yAxisID: 'y-solar'
        }
      ]
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: {
        duration: 750
      },
      interaction: {
        mode: 'index',
        intersect: false
      },
      plugins: {
        legend: {
          display: true,
          position: 'top',
          labels: {
            color: '#00d9ff',
            font: {
              size: 12,
              family: 'Poppins'
            },
            padding: 15,
            usePointStyle: true
          }
        },
        tooltip: {
          backgroundColor: 'rgba(0, 0, 0, 0.8)',
          titleColor: '#00d9ff',
          bodyColor: '#fff',
          borderColor: '#00d9ff',
          borderWidth: 1,
          padding: 12,
          displayColors: true,
          callbacks: {
            label: function(context) {
              let label = context.dataset.label || '';
              if (label) {
                label += ': ';
              }
              label += context.parsed.y.toFixed(2);
              return label;
            }
          }
        }
      },
      scales: {
        x: {
          display: true,
          grid: {
            color: 'rgba(0, 217, 255, 0.1)',
            drawBorder: false
          },
          ticks: {
            color: '#888',
            font: {
              size: 10
            },
            maxRotation: 0
          }
        },
        'y-proximity': {
          type: 'linear',
          display: true,
          position: 'left',
          title: {
            display: true,
            text: 'Fence Status',
            color: '#00d9ff',
            font: {
              size: 11
            }
          },
          grid: {
            color: 'rgba(0, 217, 255, 0.1)',
            drawBorder: false
          },
          ticks: {
            color: '#00d9ff',
            font: {
              size: 10
            }
          },
          min: 0,
          max: 50
        },
        'y-temp': {
          type: 'linear',
          display: true,
          position: 'right',
          title: {
            display: true,
            text: 'Temperature (°C)',
            color: '#ff6b6b',
            font: {
              size: 11
            }
          },
          grid: {
            drawOnChartArea: false,
            color: 'rgba(255, 107, 107, 0.1)'
          },
          ticks: {
            color: '#ff6b6b',
            font: {
              size: 10
            }
          },
          min: 20,
          max: 40
        },
        'y-percentage': {
          type: 'linear',
          display: true,
          position: 'right',
          title: {
            display: true,
            text: 'Humidity & Battery (%)',
            color: '#4ecdc4',
            font: {
              size: 11
            }
          },
          grid: {
            drawOnChartArea: false,
            color: 'rgba(78, 205, 196, 0.1)'
          },
          ticks: {
            color: '#4ecdc4',
            font: {
              size: 10
            }
          },
          min: 0,
          max: 100
        },
        'y-solar': {
          type: 'linear',
          display: true,
          position: 'right',
          title: {
            display: true,
            text: 'Solar (W)',
            color: '#ffd700',
            font: {
              size: 11
            }
          },
          grid: {
            drawOnChartArea: false,
          },
          ticks: {
            color: '#ffd700',
            font: {
              size: 10
            },
          },
          min: 0,
          max: 100
        }
      }
    }
  });
}

// ==========================================================
// === 3. MODIFIED PAGE LOAD (The entry point) ===
// ==========================================================

window.onload = function() {
  // Start local clock (will be updated by fetch)
  updateTime();
  setInterval(updateTime, 1000); 
  
  // Initialize all UI components
  initializeESP32Info();
  initializeModals();
  initializeSettings();
  initializeChart();
  
  // --- This is the new, important part ---
  // 1. Fetch data immediately on load
  fetchDataFromESP32();
  
  // 2. Set an interval to fetch data every 3 seconds
  //    (You can link this to the settings modal value)
  const refreshRate = localStorage.getItem('fenceora_settings') ? 
                      JSON.parse(localStorage.getItem('fenceora_settings')).refreshRate : 3000;
                      
  setInterval(fetchDataFromESP32, parseInt(refreshRate));
};
