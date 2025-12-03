// ============================
// Electric Spark Loading Animation Control
// ============================

window.addEventListener('load', () => {
  const loadingScreen = document.getElementById('loadingScreen');
  const mainContainer = document.getElementById('mainContainer');
  const loadingProgress = document.getElementById('loadingProgress');
  const loadingPercentage = document.getElementById('loadingPercentage');
  const thunderFlash = document.getElementById('thunderFlash');
  const electricHum = document.getElementById('electricHum');
  const thunderSound = document.getElementById('thunderSound');
  
  // Play ambient electric hum sound
  if (electricHum) {
    electricHum.volume = 0.15;
    electricHum.play().catch(e => console.log('Audio play prevented:', e));
  }
  
  let progress = 0;
  const duration = 3000; // 3 seconds
  const intervalTime = 30;
  const increment = (100 / duration) * intervalTime;
  
  // Animate loading percentage and fence progress
  const loadingInterval = setInterval(() => {
    progress += increment;
    
    if (progress >= 100) {
      progress = 100;
      clearInterval(loadingInterval);
      
      // Stop electric hum
      if (electricHum) {
        electricHum.pause();
      }
      
      // MASSIVE THUNDER EFFECT at 100%!
      setTimeout(() => {
        // Trigger thunder flash
        thunderFlash.classList.add('active');
        
        // Play thunder sound
        if (thunderSound) {
          thunderSound.volume = 0.4;
          thunderSound.play().catch(e => console.log('Audio play prevented:', e));
        }
        
        // Make all lightning bolts flash intensely
        const allLightning = document.querySelectorAll('.lightning');
        allLightning.forEach(bolt => {
          bolt.style.opacity = '1';
          bolt.style.animation = 'none';
        });
        
        // Flash effect
        setTimeout(() => {
          allLightning.forEach(bolt => {
            bolt.style.opacity = '0';
          });
        }, 100);
        
        setTimeout(() => {
          allLightning.forEach(bolt => {
            bolt.style.opacity = '1';
          });
        }, 200);
        
        setTimeout(() => {
          allLightning.forEach(bolt => {
            bolt.style.opacity = '0';
          });
        }, 300);
        
        setTimeout(() => {
          allLightning.forEach(bolt => {
            bolt.style.opacity = '1';
          });
        }, 450);
        
        // Shake the screen effect
        loadingScreen.style.animation = 'screenShake 0.5s ease-in-out';
        
        // Final fade out after thunder
        setTimeout(() => {
          loadingScreen.classList.add('fade-out');
          mainContainer.classList.add('show');
          
          // Remove loading screen from DOM after animation
          setTimeout(() => {
            loadingScreen.style.display = 'none';
          }, 800);
        }, 800);
        
      }, 300);
    }
    
    // Update progress bar
    loadingPercentage.textContent = `${Math.floor(progress)}%`;
  }, intervalTime);
});

// ============================
// Global Variables and Chart Setup
// ============================

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
// === LANGUAGE TRANSLATIONS ===
// ========================================================================

const translations = {
  en: {
    // Header
    deviceConnected: "Device is Connected",
    connectionLost: "Connection Lost",
    ipNotSet: "IP Not Set",
    editing: "Editing...",
    connecting: "Connecting...",
    
    // Status Cards
    currentDetected: "Current Detected!",
    noCurrent: "No Current",
    highHeatAlert: "High Heat Alert",
    elevated: "Elevated",
    optimal: "Optimal",
    justNow: "Just now",
    battery: "Battery",
    solarInput: "Solar Input",
    
    // Buttons
    edit: "Edit",
    save: "Save",
    
    // Warning Overlay
    criticalAlert: "⚠️ CRITICAL ALERT",
    fenceCurrentDetected: "⚡ FENCE CURRENT DETECTED",
    electricFieldDetected: "Electric field detected on perimeter fence!",
    tamperingDetected: "🚨 TAMPERING DETECTED",
    handDetected: "Hand or object detected near device!",
    deviceMovement: "⚠️ DEVICE MOVEMENT DETECTED",
    someoneTryingMove: "Someone is trying to move the device!",
    unauthorizedAccess: "🔒 UNAUTHORIZED ACCESS ATTEMPT",
    unknownCard: "Unknown RFID card scanned!",
    dismiss: "DISMISS",
    
    // Time & Sensor Labels
    time: "Time",
    voltage: "Voltage",
    sensor: "Sensor",
    infrared: "Infrared",
    accelerometer: "MPU6050 Accelerometer",
    capacitive: "Capacitive",
    cardUID: "Card UID"
  },
  ta: {
    // Header (Tamil)
    deviceConnected: "சாதனம் இணைக்கப்பட்டுள்ளது",
    connectionLost: "இணைப்பு துண்டிக்கப்பட்டது",
    ipNotSet: "IP அமைக்கப்படவில்லை",
    editing: "திருத்துகிறது...",
    connecting: "இணைக்கிறது...",
    
    // Status Cards (Tamil)
    currentDetected: "மின்னோட்டம் கண்டறியப்பட்டது!",
    noCurrent: "மின்னோட்டம் இல்லை",
    highHeatAlert: "அதிக வெப்ப எச்சரிக்கை",
    elevated: "உயர்ந்த",
    optimal: "உகந்த",
    justNow: "இப்போது",
    battery: "பேட்டரி",
    solarInput: "சூரிய உள்ளீடு",
    
    // Buttons (Tamil)
    edit: "திருத்து",
    save: "சேமி",
    
    // Warning Overlay (Tamil)
    criticalAlert: "⚠️ முக்கிய எச்சரிக்கை",
    fenceCurrentDetected: "⚡ வேலி மின்னோட்டம் கண்டறியப்பட்டது",
    electricFieldDetected: "சுற்றளவு வேலியில் மின்புலம் கண்டறியப்பட்டது!",
    tamperingDetected: "🚨 சிதைவு கண்டறியப்பட்டது",
    handDetected: "சாதனத்திற்கு அருகில் கை அல்லது பொருள் கண்டறியப்பட்டது!",
    deviceMovement: "⚠️ சாதன இயக்கம் கண்டறியப்பட்டது",
    someoneTryingMove: "யாரோ சாதனத்தை நகர்த்த முயற்சிக்கிறார்கள்!",
    unauthorizedAccess: "🔒 அங்கீகரிக்கப்படாத அணுகல் முயற்சி",
    unknownCard: "அறியப்படாத RFID கார்டு ஸ்கேன் செய்யப்பட்டது!",
    dismiss: "மூடு",
    
    // Time & Sensor Labels (Tamil)
    time: "நேரம்",
    voltage: "மின்னழுத்தம்",
    sensor: "உணரி",
    infrared: "அகச்சிவப்பு",
    accelerometer: "MPU6050 முடுக்கமானி",
    capacitive: "கொள்ளளவு",
    cardUID: "கார்டு UID"
  }
};

let currentLanguage = 'en'; // Default language

// Helper function to get translated text
function t(key) {
  return translations[currentLanguage][key] || translations.en[key] || key;
}

// Helper function to convert Celsius to Fahrenheit
function celsiusToFahrenheit(celsius) {
  return (celsius * 9/5) + 32;
}

// Helper function to get temperature with unit
function formatTemperature(celsius) {
  const settings = JSON.parse(localStorage.getItem('fenceora_settings') || '{}');
  const tempUnit = settings.tempUnit || 'C';
  
  if (tempUnit === 'F') {
    const fahrenheit = celsiusToFahrenheit(celsius);
    return fahrenheit.toFixed(1) + '°F';
  }
  return celsius.toFixed(1) + '°C';
}

// Helper function to get temperature threshold based on unit
function getTempThreshold(celsiusThreshold) {
  const settings = JSON.parse(localStorage.getItem('fenceora_settings') || '{}');
  const tempUnit = settings.tempUnit || 'C';
  
  if (tempUnit === 'F') {
    return celsiusToFahrenheit(celsiusThreshold);
  }
  return celsiusThreshold;
}

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
    document.getElementById('connectionText').textContent = t('ipNotSet');
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
    updateMPU6050(data); // Update MPU6050 sensor display
    
    // Check for critical alerts (fence current, tampering, etc.)
    checkForCriticalAlerts(data);

    // Update connection status to "Connected"
    const connectionDot = document.querySelector('.connection-dot');
    document.getElementById('connectionText').textContent = t('deviceConnected');
    connectionDot.style.backgroundColor = 'var(--secondary)';
    connectionDot.classList.add('connected');

  } catch (error) {
    console.error('Fetch error:', error);
    // Update connection status to "Connection Lost"
    const connectionDot = document.querySelector('.connection-dot');
    document.getElementById('connectionText').textContent = t('connectionLost');
    connectionDot.style.backgroundColor = 'var(--danger)';
    connectionDot.classList.remove('connected');
  }
}

// Previous sensor states for change detection
let previousStates = {
  capacitiveDetected: false,
  currentDetected: false,
  irDetected: false,
  tamperDetected: false,
  ultrasonicCritical: false
};

/**
 * Updates the Capacitive and Temperature sensor cards
 */
function updateStatusCards(data) {
  // 1. Capacitive Proximity Sensor (Electric Field Detection)
  const proximityValue = document.getElementById('proxValue');
  const proximityStatus = document.getElementById('proxStatus');
  const proximityUpdate = document.getElementById('proxUpdate');
  
  // Use capacitiveDetected from new ESP32 API
  if (data.capacitiveDetected) {
    proximityValue.textContent = "ELECTRIC FIELD DETECTED";
    proximityStatus.textContent = t('currentDetected');
    proximityStatus.className = "status-badge status-alert";
  } else {
    proximityValue.textContent = "CLEAR";
    proximityStatus.textContent = t('noCurrent');
    proximityStatus.className = "status-badge status-normal";
  }
  proximityUpdate.textContent = t('justNow');
  
  // 2. Temperature Sensor with Unit Conversion
  const temperatureValue = document.getElementById('tempValue');
  const temperatureStatus = document.getElementById('tempStatus');
  
  // Convert temperature based on settings
  temperatureValue.textContent = formatTemperature(data.temperature.value);
  
  // Temperature thresholds (using converted values)
  const highTemp = getTempThreshold(35);
  const elevatedTemp = getTempThreshold(30);
  const currentTemp = getTempThreshold(data.temperature.value);
  
  if (currentTemp > highTemp) {
    temperatureStatus.textContent = t('highHeatAlert');
    temperatureStatus.className = "status-badge status-alert";
  } else if (currentTemp > elevatedTemp) {
    temperatureStatus.textContent = t('elevated');
    temperatureStatus.className = "status-badge status-warning";
  } else {
    temperatureStatus.textContent = t('optimal');
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
  batteryText.textContent = `${t('battery')}: ${Math.round(newWidth)}%`;

  // 3. Solar
  const solarInput = document.querySelector('.battery-container .detail-label');
  const solarPower = document.querySelector('.solar-container .detail-label');
  
  if(solarInput) solarInput.textContent = `${t('solarInput')}: ${data.solar.voltage.toFixed(1)}V`;
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
    connectionText.textContent = t('connecting');
    connectionDot.style.backgroundColor = 'var(--warning)';
  } else {
    // No saved IP - ready for user input
    ipInput.disabled = false;
    ipInput.focus();
    editBtn.classList.add('hidden');
    saveBtn.classList.remove('hidden');
    connectionText.textContent = t('editing');
    connectionDot.style.backgroundColor = 'var(--warning)';
  }
  
  // Edit button click handler
  editBtn.addEventListener('click', function() {
    ipInput.disabled = false;
    ipInput.focus();
    ipInput.select();
    editBtn.classList.add('hidden');
    saveBtn.classList.remove('hidden');
    connectionText.textContent = t('editing');
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
      connectionText.textContent = t('connecting');
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

// Settings functionality with Theme Toggle
function initializeSettings() {
  const saveBtn = document.querySelector('.btn-save');
  const resetBtn = document.querySelector('.btn-reset');
  const themeSelect = document.getElementById('theme');
  const languageSelect = document.getElementById('language');
  const tempUnitSelect = document.getElementById('tempUnit');
  const emailNotificationsCheckbox = document.getElementById('emailNotifications');
  const emailTestSection = document.getElementById('emailTestSection');
  const testEmailBtn = document.getElementById('testEmailBtn');
  
  let testEmailTimer = null;
  let testEmailCooldown = 0;

  // Show/hide email test section based on checkbox
  if (emailNotificationsCheckbox && emailTestSection) {
    // Show by default if checked
    if (emailNotificationsCheckbox.checked) {
      emailTestSection.style.display = 'block';
    }
    
    emailNotificationsCheckbox.addEventListener('change', function() {
      if (this.checked) {
        emailTestSection.style.display = 'block';
      } else {
        emailTestSection.style.display = 'none';
      }
    });
  }

  // Test Email Button Handler
  if (testEmailBtn) {
    testEmailBtn.addEventListener('click', async function() {
      if (testEmailCooldown > 0) return;
      
      // Disable button and show loading
      testEmailBtn.disabled = true;
      const testEmailText = document.getElementById('testEmailText');
      const originalText = testEmailText.textContent;
      testEmailText.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Sending...';

      try {
        // Send test email using a email service (EmailJS, FormSubmit, etc.)
        // For now, we'll simulate the email sending
        const result = await sendTestEmail();
        
        if (result && result.responses && result.responses.length > 0) {
          showEmailStatus(`✅ Test emails sent to all ${result.responses.length} receivers! Check your inboxes (and spam folder).`, 'success');
        } else {
          showEmailStatus('✅ Test email sent successfully! (Simulation mode - Check email-config.js to enable real emails)', 'success');
        }
        startTestEmailCooldown();
      } catch (error) {
        console.error('Test email error:', error);
        showEmailStatus(`❌ Error: ${error.message}`, 'error');
        testEmailBtn.disabled = false;
        testEmailText.innerHTML = '<i class="fas fa-paper-plane"></i> Send Test Email';
      }
    });
  }

  // NOTE: sendTestEmail() function is loaded from email-sender.js

  // Start 30-second cooldown timer
  function startTestEmailCooldown() {
    testEmailCooldown = 30;
    const testEmailText = document.getElementById('testEmailText');
    const emailTestTimer = document.getElementById('emailTestTimer');
    const timerCountdown = document.getElementById('timerCountdown');
    
    // Show timer
    emailTestTimer.classList.add('active');
    
    // Update countdown every second
    testEmailTimer = setInterval(() => {
      testEmailCooldown--;
      timerCountdown.textContent = testEmailCooldown;
      
      if (testEmailCooldown <= 0) {
        clearInterval(testEmailTimer);
        testEmailBtn.disabled = false;
        testEmailText.innerHTML = '<i class="fas fa-paper-plane"></i> Send Test Email';
        emailTestTimer.classList.remove('active');
      }
    }, 1000);
  }

  // Show email status message
  function showEmailStatus(message, type = 'success') {
    const emailTestStatus = document.getElementById('emailTestStatus');
    const emailStatusText = document.getElementById('emailStatusText');
    
    emailStatusText.textContent = message;
    emailTestStatus.className = 'email-test-status';
    if (type === 'error') {
      emailTestStatus.classList.add('error');
    }
    
    emailTestStatus.classList.add('active');
    
    // Hide status after 5 seconds for success, 8 seconds for error
    setTimeout(() => {
      emailTestStatus.classList.remove('active');
    }, type === 'error' ? 8000 : 5000);
  }

  // Apply theme immediately on change
  themeSelect.addEventListener('change', function() {
    applyTheme(this.value);
  });

  // Apply language immediately on change
  if (languageSelect) {
    languageSelect.addEventListener('change', function() {
      currentLanguage = this.value;
      applyLanguage();
      // Re-fetch data to update UI with new language
      fetchDataFromESP32();
    });
  }

  // Apply temperature unit immediately on change
  tempUnitSelect.addEventListener('change', function() {
    // Save to settings
    const settings = JSON.parse(localStorage.getItem('fenceora_settings') || '{}');
    settings.tempUnit = this.value;
    localStorage.setItem('fenceora_settings', JSON.stringify(settings));
    // Re-fetch data to update temperature display
    fetchDataFromESP32();
  });

  saveBtn.addEventListener('click', function() {
    // Get all settings values
    const settings = {
      refreshRate: document.getElementById('refreshRate').value,
      theme: document.getElementById('theme').value,
      language: languageSelect ? languageSelect.value : 'en',
      enableNotifications: document.getElementById('enableNotifications').checked,
      soundAlerts: document.getElementById('soundAlerts').checked,
      emailNotifications: document.getElementById('emailNotifications').checked,
      timeFormat: document.getElementById('timeFormat').value,
      tempUnit: document.getElementById('tempUnit').value
    };

    // Save to localStorage
    localStorage.setItem('fenceora_settings', JSON.stringify(settings));
    
    // Apply the selected theme and language
    applyTheme(settings.theme);
    currentLanguage = settings.language;
    applyLanguage();

    // Show success feedback
    saveBtn.innerHTML = '<i class="fas fa-check"></i> Saved!';
    saveBtn.style.background = 'linear-gradient(135deg, var(--secondary), var(--secondary))';

    setTimeout(() => {
      saveBtn.innerHTML = '<i class="fas fa-save"></i> Save Settings';
      saveBtn.style.background = 'linear-gradient(135deg, var(--primary), var(--secondary))';
      document.getElementById('settingsModal').classList.remove('show');
      // Refresh data to apply new settings
      fetchDataFromESP32();
    }, 1500);
  });

  resetBtn.addEventListener('click', function() {
    if (confirm('Reset all settings to default?')) {
      localStorage.removeItem('fenceora_settings');
      document.body.classList.remove('light-theme');
      currentLanguage = 'en';
      location.reload();
    }
  });

  // Load saved settings
  const savedSettings = localStorage.getItem('fenceora_settings');
  if (savedSettings) {
    const settings = JSON.parse(savedSettings);
    document.getElementById('refreshRate').value = settings.refreshRate || '5000';
    document.getElementById('theme').value = settings.theme || 'dark';
    if (languageSelect) languageSelect.value = settings.language || 'en';
    document.getElementById('enableNotifications').checked = settings.enableNotifications !== false;
    document.getElementById('soundAlerts').checked = settings.soundAlerts !== false;
    document.getElementById('emailNotifications').checked = settings.emailNotifications || false;
    document.getElementById('timeFormat').value = settings.timeFormat || '24';
    document.getElementById('tempUnit').value = settings.tempUnit || 'C';
    
    // Show email test section if email notifications are enabled
    if (settings.emailNotifications && emailTestSection) {
      emailTestSection.style.display = 'block';
    }
    
    // Apply saved theme and language on load
    applyTheme(settings.theme);
    currentLanguage = settings.language || 'en';
    applyLanguage();
  }
}

// Apply language to UI elements
function applyLanguage() {
  // Update connection text if it exists
  const connectionText = document.getElementById('connectionText');
  if (connectionText) {
    const currentText = connectionText.textContent;
    if (currentText.includes('Connected')) connectionText.textContent = t('deviceConnected');
    else if (currentText.includes('Lost')) connectionText.textContent = t('connectionLost');
    else if (currentText.includes('Not Set')) connectionText.textContent = t('ipNotSet');
    else if (currentText.includes('Editing')) connectionText.textContent = t('editing');
    else if (currentText.includes('Connecting')) connectionText.textContent = t('connecting');
  }
  
  // Update button texts
  const editBtn = document.getElementById('editIpBtn');
  const saveBtn = document.getElementById('saveIpBtn');
  if (editBtn) editBtn.textContent = t('edit');
  if (saveBtn) saveBtn.textContent = t('save');
}

// Apply theme function
function applyTheme(theme) {
  if (theme === 'light') {
    document.body.classList.add('light-theme');
  } else {
    document.body.classList.remove('light-theme');
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

// ==========================================================
// === 4. CRITICAL WARNING OVERLAY SYSTEM ===
// ==========================================================

/**
 * Shows a critical warning overlay on the dashboard
 * @param {string} title - Main warning title
 * @param {string} message - Warning message
 * @param {string} details - Additional details (time, location, etc.)
 */
function showWarning(title, message, details) {
  document.getElementById('warningTitle').textContent = title;
  document.getElementById('warningMessage').textContent = message;
  document.getElementById('warningDetails').textContent = details;
  document.getElementById('criticalWarningOverlay').classList.remove('hidden');
  
  // Play alert sound (optional - you can add an audio element)
  // const alertSound = new Audio('alert.mp3');
  // alertSound.play();
  
  // Note: Email notifications are sent directly from ESP32
  // No browser-side email code needed!
  console.log('⚠️ Critical Alert:', title, message, details);
  console.log('📧 Email notifications sent from ESP32 to all receivers');
}

/**
 * Dismisses the critical warning overlay
 */
function dismissWarning() {
  document.getElementById('criticalWarningOverlay').classList.add('hidden');
}

/**
 * Updates MPU6050 Motion Sensor card with gyroscope and accelerometer data
 * @param {object} data - ESP32 sensor data
 */
function updateMPU6050(data) {
  // Check if MPU6050 data exists
  if (!data.mpu6050) return;
  
  const mpu = data.mpu6050;
  
  // Update Gyroscope values (degrees/second)
  document.getElementById('gyroX').textContent = mpu.gyro.x.toFixed(2);
  document.getElementById('gyroY').textContent = mpu.gyro.y.toFixed(2);
  document.getElementById('gyroZ').textContent = mpu.gyro.z.toFixed(2);
  
  // Update Accelerometer values (m/s²)
  document.getElementById('accelX').textContent = mpu.accel.x.toFixed(2);
  document.getElementById('accelY').textContent = mpu.accel.y.toFixed(2);
  document.getElementById('accelZ').textContent = mpu.accel.z.toFixed(2);
  
  // Update temperature if available
  if (mpu.temperature) {
    document.getElementById('mpuTemp').textContent = `${mpu.temperature.toFixed(1)}°C`;
  }
  
  // Calculate vibration level from accelerometer magnitude
  const accelMagnitude = Math.sqrt(
    mpu.accel.x ** 2 + 
    mpu.accel.y ** 2 + 
    mpu.accel.z ** 2
  );
  
  let vibrationLevel = 'Low';
  let vibrationColor = 'var(--secondary)';
  
  if (accelMagnitude > 15) {
    vibrationLevel = 'High';
    vibrationColor = 'var(--danger)';
  } else if (accelMagnitude > 11) {
    vibrationLevel = 'Medium';
    vibrationColor = 'var(--warning)';
  }
  
  const vibrationElement = document.getElementById('vibrationLevel');
  vibrationElement.textContent = vibrationLevel;
  vibrationElement.style.color = vibrationColor;
  
  // Update status badges based on motion detection
  const mpuStatusBadge = document.getElementById('mpuStatus');
  const mpuStatusText = document.getElementById('mpuStatusText');
  const mpuAlertBadge = document.getElementById('mpuAlert');
  
  // Detect significant motion (gyroscope threshold)
  const gyroMagnitude = Math.sqrt(
    mpu.gyro.x ** 2 + 
    mpu.gyro.y ** 2 + 
    mpu.gyro.z ** 2
  );
  
  if (gyroMagnitude > 50 || accelMagnitude > 15) {
    // Motion detected!
    mpuStatusText.textContent = 'Motion Detected';
    mpuStatusBadge.style.background = 'rgba(255, 71, 87, 0.2)';
    mpuStatusBadge.style.borderColor = 'var(--danger)';
    mpuStatusBadge.style.color = 'var(--danger)';
    mpuAlertBadge.classList.add('active');
  } else {
    // Stable
    mpuStatusText.textContent = 'Stable';
    mpuStatusBadge.style.background = 'rgba(0, 255, 174, 0.15)';
    mpuStatusBadge.style.borderColor = 'var(--secondary)';
    mpuStatusBadge.style.color = 'var(--secondary)';
    mpuAlertBadge.classList.remove('active');
  }
}

/**
 * Checks for critical alerts in the data and triggers warnings
 * Called from fetchDataFromESP32() after receiving data
 * @param {object} data - ESP32 sensor data
 */
function checkForCriticalAlerts(data) {
  const now = new Date().toLocaleTimeString();
  const settings = JSON.parse(localStorage.getItem('fenceora_settings') || '{}');
  
  // === CRITICAL: Only trigger notification on STATE CHANGE (Normal → Detected) ===
  
  // 1. Check CAPACITIVE SENSOR (Electric Field) - State Change Detection
  if (data.capacitiveDetected && !previousStates.capacitiveDetected) {
    console.log('🔔 CAPACITIVE: Normal → Detected (Triggering notification)');
    showWarning(
      '⚡ CAPACITIVE SENSOR ALERT',
      'Electric field detected on fence!',
      `${t('time')}: ${now} | Sensor: Capacitive`
    );
    
    if (settings.emailNotifications) {
      sendAlertEmail('Capacitive Sensor Alert', 
        `⚡ CRITICAL: Electric field detected on perimeter fence! Time: ${now}`);
    }
  }
  previousStates.capacitiveDetected = data.capacitiveDetected;
  
  // 2. Check CURRENT SENSOR (ACS712) - State Change Detection
  if (data.currentDetected && !previousStates.currentDetected) {
    console.log('🔔 CURRENT: Normal → Detected (Triggering notification)');
    showWarning(
      '⚡ HIGH CURRENT DETECTED',
      'Abnormal current flow detected!',
      `${t('time')}: ${now} | Sensor: ACS712`
    );
    
    if (settings.emailNotifications) {
      sendAlertEmail('High Current Alert', 
        `⚡ CRITICAL: High current detected in system! Time: ${now}`);
    }
  }
  previousStates.currentDetected = data.currentDetected;
  
  // 3. Check IR SENSOR (Tampering) - State Change Detection
  if (data.irDetected && !previousStates.irDetected) {
    console.log('🔔 IR: Normal → Detected (Triggering notification)');
    showWarning(
      '🚨 TAMPERING DETECTED',
      'Hand or object detected near device!',
      `${t('time')}: ${now} | Sensor: Infrared`
    );
    
    if (settings.emailNotifications) {
      sendAlertEmail('Tampering Detected', 
        `🚨 ALERT: Hand or object detected near device! Time: ${now}`);
    }
  }
  previousStates.irDetected = data.irDetected;
  
  // 4. Check TAMPER/GYROSCOPE (Motion) - State Change Detection
  if (data.tamperDetected && !previousStates.tamperDetected) {
    console.log('🔔 TAMPER: Normal → Detected (Triggering notification)');
    showWarning(
      '⚠️ DEVICE MOVEMENT',
      'Someone is trying to move the device!',
      `${t('time')}: ${now} | Sensor: MPU6050 Gyroscope`
    );
    
    if (settings.emailNotifications) {
      sendAlertEmail('Device Movement Detected', 
        `⚠️ WARNING: Device movement detected! Time: ${now}`);
    }
  }
  previousStates.tamperDetected = data.tamperDetected;
  
  // 5. Check ULTRASONIC (Critical Proximity) - State Change Detection
  const ultrasonicCritical = (data.ultrasonic > 0 && data.ultrasonic < 30);
  if (ultrasonicCritical && !previousStates.ultrasonicCritical) {
    console.log('🔔 ULTRASONIC: Normal → Critical (Triggering notification)');
    showWarning(
      '🚨 CRITICAL PROXIMITY',
      `Object too close: ${data.ultrasonic} cm`,
      `${t('time')}: ${now} | Sensor: Ultrasonic`
    );
    
    if (settings.emailNotifications) {
      sendAlertEmail('Proximity Alert', 
        `🚨 CRITICAL: Object detected at ${data.ultrasonic} cm! Time: ${now}`);
    }
  }
  previousStates.ultrasonicCritical = ultrasonicCritical;
}

// Send alert email to all receivers
async function sendAlertEmail(alertType, alertMessage) {
  if (typeof EMAIL_CONFIG === 'undefined' || !EMAIL_CONFIG.emailjs.enabled) {
    console.log('Email notifications not configured');
    return;
  }
  
  try {
    emailjs.init(EMAIL_CONFIG.emailjs.publicKey);
    
    const emailPromises = EMAIL_CONFIG.recipients.receivers.map(async (receiverEmail) => {
      const templateParams = {
        to_email: receiverEmail,
        to_name: receiverEmail.split('@')[0],
        from_name: 'FENCEORA Security System',
        subject: `🚨 FENCEORA ALERT: ${alertType}`,
        message: alertMessage,
        timestamp: new Date().toLocaleString(),
        system_status: 'ALERT - Immediate Attention Required',
        alert_type: alertType,
        sender_email: EMAIL_CONFIG.recipients.sender,
        receiver_number: receiverEmail
      };
      
      return emailjs.send(
        EMAIL_CONFIG.emailjs.serviceId,
        EMAIL_CONFIG.emailjs.templateId,
        templateParams
      );
    });
    
    await Promise.all(emailPromises);
    console.log(`✅ Alert emails sent: ${alertType}`);
  } catch (error) {
    console.error('❌ Failed to send alert email:', error);
  }
}

// ============================
// Floating Feedback Button & Modal
// ============================

// Feedback Modal
const feedbackBtn = document.getElementById('feedbackBtn');
const feedbackModal = document.getElementById('feedbackModal');
const closeFeedbackModal = document.getElementById('closeFeedbackModal');
const cancelFeedback = document.getElementById('cancelFeedback');
const feedbackForm = document.getElementById('feedbackForm');

if (feedbackBtn && feedbackModal) {
  feedbackBtn.addEventListener('click', () => {
    feedbackModal.style.display = 'flex';
  });

  closeFeedbackModal.addEventListener('click', () => {
    feedbackModal.style.display = 'none';
    feedbackForm.reset();
  });

  cancelFeedback.addEventListener('click', () => {
    feedbackModal.style.display = 'none';
    feedbackForm.reset();
  });

  window.addEventListener('click', (e) => {
    if (e.target === feedbackModal) {
      feedbackModal.style.display = 'none';
      feedbackForm.reset();
    }
  });
}

// Feedback Form Submission
if (feedbackForm) {
  feedbackForm.addEventListener('submit', async (e) => {
    e.preventDefault();

    const name = document.getElementById('feedbackName').value;
    const email = document.getElementById('feedbackEmail').value;
    const type = document.getElementById('feedbackType').value;
    const message = document.getElementById('feedbackMessage').value;

    // Disable submit button
    const submitBtn = feedbackForm.querySelector('.btn-submit-feedback');
    const originalBtnText = submitBtn.innerHTML;
    submitBtn.disabled = true;
    submitBtn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Sending...';

    try {
      // Check if EmailJS is configured
      if (typeof EMAIL_CONFIG === 'undefined' || !EMAIL_CONFIG.emailjs.enabled) {
        throw new Error('Email service not configured');
      }

      // Initialize EmailJS
      emailjs.init(EMAIL_CONFIG.emailjs.publicKey);

      // Send feedback email to fenceora.h4@gmail.com
      const templateParams = {
        to_email: 'fenceora.h4@gmail.com',
        to_name: 'FENCEORA Team',
        from_name: name,
        sender_email: email,
        subject: `📩 FENCEORA Feedback: ${type}`,
        message: message,
        feedback_type: type,
        timestamp: new Date().toLocaleString(),
        system_status: 'User Feedback Submission',
        alert_type: type,
        receiver_number: 'fenceora.h4@gmail.com'
      };

      await emailjs.send(
        EMAIL_CONFIG.emailjs.serviceId,
        EMAIL_CONFIG.emailjs.templateId,
        templateParams
      );

      // Show success message
      showFeedbackStatus('✅ Thank you! Your feedback has been sent successfully!', 'success');
      
      // Reset form and close modal after 3 seconds
      setTimeout(() => {
        feedbackForm.reset();
        feedbackModal.style.display = 'none';
        document.getElementById('feedbackStatus').classList.remove('active');
      }, 3000);

      console.log('✅ Feedback sent successfully');
    } catch (error) {
      console.error('❌ Failed to send feedback:', error);
      showFeedbackStatus('❌ Failed to send feedback. Please try again or email us directly at fenceora.h4@gmail.com', 'error');
    } finally {
      // Re-enable submit button
      submitBtn.disabled = false;
      submitBtn.innerHTML = originalBtnText;
    }
  });
}

// Show feedback status message
function showFeedbackStatus(message, type = 'success') {
  const feedbackStatus = document.getElementById('feedbackStatus');
  const feedbackStatusText = document.getElementById('feedbackStatusText');
  
  if (feedbackStatus && feedbackStatusText) {
    feedbackStatusText.textContent = message;
    feedbackStatus.className = 'feedback-status';
    if (type === 'error') {
      feedbackStatus.classList.add('error');
    }
    feedbackStatus.classList.add('active');
  }
}

// ============================
// Emoji Eyes Follow Cursor - SIMPLIFIED
// ============================
window.addEventListener('load', function() {
  console.log('🔍 Starting eye tracking initialization...');
  
  setTimeout(() => {
    const leftPupil = document.querySelector('.left-eye .emoji-pupil');
    const rightPupil = document.querySelector('.right-eye .emoji-pupil');
    const leftEye = document.querySelector('.left-eye');
    const rightEye = document.querySelector('.right-eye');
    
    console.log('Left pupil found:', leftPupil);
    console.log('Right pupil found:', rightPupil);
    console.log('Left eye found:', leftEye);
    console.log('Right eye found:', rightEye);
    
    if (!leftPupil || !rightPupil || !leftEye || !rightEye) {
      console.error('❌ Emoji elements not found!');
      return;
    }
    
    console.log('✅ All emoji elements found! Starting tracking...');
    
    document.addEventListener('mousemove', function(e) {
      try {
        // Get left eye position
        const leftEyeRect = leftEye.getBoundingClientRect();
        const leftEyeCenterX = leftEyeRect.left + leftEyeRect.width / 2;
        const leftEyeCenterY = leftEyeRect.top + leftEyeRect.height / 2;
        
        // Get right eye position
        const rightEyeRect = rightEye.getBoundingClientRect();
        const rightEyeCenterX = rightEyeRect.left + rightEyeRect.width / 2;
        const rightEyeCenterY = rightEyeRect.top + rightEyeRect.height / 2;
        
        // Calculate for left eye
        const leftDeltaX = e.clientX - leftEyeCenterX;
        const leftDeltaY = e.clientY - leftEyeCenterY;
        const leftAngle = Math.atan2(leftDeltaY, leftDeltaX);
        const leftDistance = Math.min(2, Math.sqrt(leftDeltaX * leftDeltaX + leftDeltaY * leftDeltaY) / 200);
        
        const leftPupilX = Math.cos(leftAngle) * leftDistance;
        const leftPupilY = Math.sin(leftAngle) * leftDistance;
        
        // Calculate for right eye
        const rightDeltaX = e.clientX - rightEyeCenterX;
        const rightDeltaY = e.clientY - rightEyeCenterY;
        const rightAngle = Math.atan2(rightDeltaY, rightDeltaX);
        const rightDistance = Math.min(2, Math.sqrt(rightDeltaX * rightDeltaX + rightDeltaY * rightDeltaY) / 200);
        
        const rightPupilX = Math.cos(rightAngle) * rightDistance;
        const rightPupilY = Math.sin(rightAngle) * rightDistance;
        
        // Apply transform
        leftPupil.style.transform = 'translate(' + leftPupilX + 'px, ' + leftPupilY + 'px)';
        rightPupil.style.transform = 'translate(' + rightPupilX + 'px, ' + rightPupilY + 'px)';
      } catch (error) {
        console.error('Error in eye tracking:', error);
      }
    });
    
    console.log('👁️ Eye tracking is now active!');
  }, 500);
});

// ============================
// Copy Code Button Functionality
// ============================
document.addEventListener('DOMContentLoaded', function() {
  const copyButtons = document.querySelectorAll('.copy-code-btn');
  
  copyButtons.forEach(button => {
    button.addEventListener('click', function() {
      // Find the code block within the same parent
      const codeBlock = this.parentElement.querySelector('code');
      
      if (codeBlock) {
        // Get the text content
        const codeText = codeBlock.textContent;
        
        // Copy to clipboard
        navigator.clipboard.writeText(codeText).then(() => {
          // Success feedback
          const originalHTML = this.innerHTML;
          this.innerHTML = '<i class="fas fa-check"></i> Copied!';
          this.classList.add('copied');
          
          // Reset button after 2 seconds
          setTimeout(() => {
            this.innerHTML = originalHTML;
            this.classList.remove('copied');
          }, 2000);
        }).catch(err => {
          console.error('Failed to copy code:', err);
          // Fallback for older browsers
          const textArea = document.createElement('textarea');
          textArea.value = codeText;
          textArea.style.position = 'fixed';
          textArea.style.left = '-999999px';
          document.body.appendChild(textArea);
          textArea.select();
          
          try {
            document.execCommand('copy');
            const originalHTML = this.innerHTML;
            this.innerHTML = '<i class="fas fa-check"></i> Copied!';
            this.classList.add('copied');
            
            setTimeout(() => {
              this.innerHTML = originalHTML;
              this.classList.remove('copied');
            }, 2000);
          } catch (err) {
            console.error('Fallback copy failed:', err);
            alert('Failed to copy code. Please select and copy manually.');
          }
          
          document.body.removeChild(textArea);
        });
      }
    });
  });
});



