# FENCEORA - Smart Electric Fence Dashboard

A modern, responsive web dashboard for monitoring and controlling smart electric fence systems with ESP32 integration.

## Features

### 🎨 Modern UI Design
- Sleek dark theme with cyan and green accent colors
- Smooth animations and transitions
- Fully responsive design for all devices
- Real-time data visualization

### 📊 Dashboard Components

1. **Header Section**
   - Centered FENCEORA branding with glowing effect
   - ESP32 IP address display (left corner)
   - System operational status (right corner)

2. **Sensor Monitoring**
   - **Capacitive Proximity Sensor**: Distance detection with status alerts
   - **Temperature Sensor**: Real-time temperature monitoring with humidity
   - **Ultrasonic & IR Detection Log**: Motion and object detection events
   - **RFID & Servo Motor Log**: Access control and gate operation tracking
   - **RTC Time & Solar Power**: Current time, battery level, and solar power status

## File Structure

```
DTWEBSITE/
├── index.html          # Main HTML structure
├── styles.css          # All styling and animations
├── script.js           # JavaScript functionality
└── README.md           # Project documentation
```

## Technologies Used

- **HTML5**: Semantic markup
- **CSS3**: Modern styling with CSS Grid, Flexbox, animations
- **JavaScript**: ES6+ for dynamic functionality
- **Font Awesome 6.4.0**: Icon library
- **Google Fonts**: Poppins font family

## Setup Instructions

1. Place all files in the same directory (the parent `DTwebsite` folder).
2. Open `index.html` in a web browser.
3. To test with an ESP32, set the device IP in the UI and ensure the ESP32 serves sensor endpoints.

## Credits

PROJECT DONE BY H^4 HYPERS
