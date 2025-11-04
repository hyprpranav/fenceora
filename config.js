// Configuration file for FENCEORA Dashboard
// Modify these values to customize your dashboard

const CONFIG = {
  // ESP32 Configuration
  esp32: {
    defaultIP: '192.168.1.100',
    apiEndpoint: '/api/sensors', // Your ESP32 API endpoint
    updateInterval: 5000, // Update interval in milliseconds
  },

  // Sensor Configuration
  sensors: {
    proximity: {
      min: 0,
      max: 50,
      unit: 'cm',
      warningThreshold: 10,
      dangerThreshold: 5,
    },
    temperature: {
      min: 15,
      max: 40,
      unit: '°C',
      warningThreshold: 30,
      dangerThreshold: 35,
    },
    battery: {
      min: 20,
      max: 100,
      warningThreshold: 30,
      dangerThreshold: 20,
    },
  },

  // UI Configuration
  ui: {
    maxLogEntries: 8,
    animationSpeed: 300, // milliseconds
    theme: {
      primary: '#00d9ff',
      secondary: '#00ffae',
      danger: '#ff4757',
      warning: '#ffd700',
    },
  },

  // System Information
  system: {
    name: 'FENCEORA',
    version: '1.0.0',
    developer: 'H^4 HYPERS',
  },
};

// Export for use in other scripts
if (typeof module !== 'undefined' && module.exports) {
  module.exports = CONFIG;
}
