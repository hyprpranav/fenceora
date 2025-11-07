# Email Notification Setup Guide

## Overview
The Fenceora dashboard now includes email notifications for critical alerts:
- ⚡ Fence current detection
- 🚨 Tampering (IR sensor triggered)
- ⚠️ Device movement (accelerometer)
- 🔒 Unauthorized RFID attempts

When any of these events occur, you'll receive:
1. **Huge warning popup** on the dashboard (fullscreen overlay)
2. **Email notification** to your configured email address
3. **Notification corner alert** (existing log system)

---

## Setup Steps

### Option 1: Using EmailJS (Recommended - Free & Easy)

EmailJS allows you to send emails directly from JavaScript without a backend server.

#### Step 1: Create EmailJS Account
1. Go to [https://www.emailjs.com/](https://www.emailjs.com/)
2. Click **"Sign Up"** and create a free account
3. Verify your email address

#### Step 2: Add Email Service
1. In EmailJS dashboard, go to **"Email Services"**
2. Click **"Add New Service"**
3. Choose your email provider (Gmail, Outlook, etc.)
4. Follow the setup wizard to connect your email account
5. **Copy the Service ID** (e.g., `service_abc1234`)

#### Step 3: Create Email Template
1. Go to **"Email Templates"** in EmailJS dashboard
2. Click **"Create New Template"**
3. Use this template:

**Subject:**
```
🚨 FENCEORA ALERT: {{alert_title}}
```

**Body:**
```
CRITICAL ALERT FROM FENCEORA SYSTEM

Alert Type: {{alert_title}}
Message: {{alert_message}}
Details: {{alert_details}}
Timestamp: {{timestamp}}

Please check your Fenceora dashboard immediately.

---
This is an automated alert from your Fenceora Smart Fence System.
```

4. Click **"Save"** and **copy the Template ID** (e.g., `template_xyz5678`)

#### Step 4: Get Your User ID
1. Go to **"Account"** in EmailJS dashboard
2. Find **"User ID"** (or "Public Key")
3. **Copy the User ID** (e.g., `user_pqr9012`)

#### Step 5: Configure Fenceora Dashboard
1. Open `script.js` in your code editor
2. Find the `sendEmailNotification()` function (around line 755)
3. Replace these values:

```javascript
const emailConfig = {
  serviceID: 'service_abc1234', // Your Service ID from Step 2
  templateID: 'template_xyz5678', // Your Template ID from Step 3
  userID: 'user_pqr9012' // Your User ID from Step 4
};

// Email template parameters
const templateParams = {
  to_email: 'your-email@example.com', // Your email address
  alert_title: title,
  alert_message: message,
  alert_details: details,
  timestamp: new Date().toLocaleString()
};
```

4. **Uncomment the EmailJS send code** (lines 778-784):

**BEFORE:**
```javascript
// Send email using EmailJS
// Uncomment when EmailJS is configured:
/*
emailjs.send(emailConfig.serviceID, emailConfig.templateID, templateParams, emailConfig.userID)
  .then(function(response) {
    console.log('✅ Email notification sent successfully:', response);
  }, function(error) {
    console.error('❌ Email notification failed:', error);
  });
*/
```

**AFTER:**
```javascript
// Send email using EmailJS
emailjs.send(emailConfig.serviceID, emailConfig.templateID, templateParams, emailConfig.userID)
  .then(function(response) {
    console.log('✅ Email notification sent successfully:', response);
  }, function(error) {
    console.error('❌ Email notification failed:', error);
  });
```

5. Save the file

#### Step 6: Test Email Notifications
1. Open your dashboard in Five Server
2. Trigger the capacitive sensor on your ESP32
3. You should see:
   - **Warning popup** on the dashboard
   - **Email notification** in your inbox (within 1-2 seconds)

---

### Option 2: Using ESP32 SMTP (Alternative)

If you prefer to send emails directly from the ESP32 (without relying on browser), you can use the ESP32 Mail Client library.

#### Pros:
- Works even when dashboard is closed
- More reliable (no browser dependency)

#### Cons:
- Requires storing email credentials on ESP32
- More complex setup
- May not work with Gmail (requires "less secure apps")

#### Setup:
1. Install **ESP32 Mail Client** library in Arduino IDE:
   - Go to **Sketch → Include Library → Manage Libraries**
   - Search for "ESP32 Mail Client"
   - Install version 3.4.16 or newer

2. Add this code to your `Fenceora_V3_FIXED.ino`:

```cpp
#include <ESP_Mail_Client.h>

// SMTP Configuration
#define SMTP_HOST "smtp.gmail.com"  // Gmail SMTP server
#define SMTP_PORT 465               // SSL port
#define AUTHOR_EMAIL "your-email@gmail.com"
#define AUTHOR_PASSWORD "your-app-password"  // Use App Password, not regular password
#define RECIPIENT_EMAIL "your-email@gmail.com"

SMTPSession smtp;

void sendEmailAlert(String title, String message) {
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;

  SMTP_Message email;
  email.sender.name = "Fenceora System";
  email.sender.email = AUTHOR_EMAIL;
  email.subject = "🚨 FENCEORA ALERT: " + title;
  email.addRecipient("Admin", RECIPIENT_EMAIL);

  String htmlMsg = "<h2>" + title + "</h2><p>" + message + "</p>";
  email.html.content = htmlMsg.c_str();

  if (!smtp.connect(&session))
    return;

  if (!MailClient.sendMail(&smtp, &email))
    Serial.println("Error sending Email: " + smtp.errorReason());
}
```

3. Call `sendEmailAlert()` when an event occurs:

```cpp
if (fenceActive) {
  addDetectLog("fa-bolt", "⚡ FENCE ELECTRIC FIELD DETECTED!");
  sendEmailAlert("Fence Current Detected", "Electric field detected on perimeter fence!");
}
```

**Note:** For Gmail, you need to create an **App Password**:
1. Go to Google Account → Security
2. Enable 2-Step Verification
3. Go to App Passwords
4. Generate a password for "Mail"
5. Use that password in `AUTHOR_PASSWORD`

---

## How the Warning System Works

### 1. Critical Alert Triggers
The dashboard checks for these conditions in `checkForCriticalAlerts()`:

```javascript
// Fence current detection
if (data.fence.status === "ACTIVE") {
  showWarning("⚡ FENCE CURRENT DETECTED", ...);
}

// IR tampering
if (data.tamper.ir === true) {
  showWarning("🚨 TAMPERING DETECTED", ...);
}

// Device movement
if (data.tamper.motion === true) {
  showWarning("⚠️ DEVICE MOVEMENT DETECTED", ...);
}

// Unauthorized RFID
if (data.lastRfidStatus === "DENIED") {
  showWarning("🔒 UNAUTHORIZED ACCESS ATTEMPT", ...);
}
```

### 2. Warning Overlay Display
When triggered, `showWarning()` displays a fullscreen overlay with:
- **Animated warning icon** (shaking triangle)
- **Large title** (48px font)
- **Warning message** (28px font)
- **Details** (time, voltage, sensor info)
- **Dismiss button** (closes the overlay)
- **Pulsing red background** (attention-grabbing)

### 3. Email Notification
Simultaneously, `sendEmailNotification()` sends an email using EmailJS with:
- Alert title
- Alert message
- Alert details
- Timestamp

### 4. Notification Corner
The existing log system also displays the alert in the notification corner.

---

## Customization

### Change Warning Colors
In `styles.css` (line 1786):
```css
.critical-warning-box {
  background: linear-gradient(135deg, #ff4757, #ff6348); /* Red gradient */
  /* Change to orange: linear-gradient(135deg, #ff9500, #ffad33); */
  /* Change to yellow: linear-gradient(135deg, #ffd700, #ffed4e); */
}
```

### Change Animation Speed
In `styles.css` (line 1819):
```css
animation: shake 0.5s ease-in-out infinite; /* Change 0.5s to 1s for slower shake */
```

### Disable Email for Specific Alerts
In `script.js`, modify `checkForCriticalAlerts()`:
```javascript
// Don't send email for RFID denials
if (data.lastRfidStatus === "DENIED") {
  showWarning("🔒 UNAUTHORIZED ACCESS ATTEMPT", ...);
  // Remove return; to skip email
  return; // This line triggers email
}
```

### Add Sound Alert
1. Download an alert sound (e.g., `alert.mp3`)
2. Place it in your project folder
3. Uncomment these lines in `showWarning()`:
```javascript
const alertSound = new Audio('alert.mp3');
alertSound.play();
```

---

## Troubleshooting

### Issue: No email received
**Check:**
1. EmailJS User ID, Service ID, and Template ID are correct
2. EmailJS send code is uncommented
3. Browser console for errors (F12)
4. EmailJS dashboard → Usage (check if email was sent)
5. Spam/junk folder

### Issue: Warning doesn't appear
**Check:**
1. ESP32 is sending correct data format (`fence.status === "ACTIVE"`)
2. Browser console for errors
3. `criticalWarningOverlay` element exists in HTML
4. CSS file is loaded

### Issue: Warning appears multiple times
**Solution:** Add a cooldown timer:
```javascript
let lastAlertTime = 0;
const ALERT_COOLDOWN = 30000; // 30 seconds

function checkForCriticalAlerts(data) {
  const now = Date.now();
  if (now - lastAlertTime < ALERT_COOLDOWN) {
    return; // Skip if alert was shown recently
  }
  
  if (data.fence.status === "ACTIVE") {
    showWarning(...);
    lastAlertTime = now;
  }
}
```

---

## EmailJS Free Tier Limits
- **200 emails/month** (free plan)
- Upgrade to **Essential plan** ($9/month) for 1,000 emails
- Each alert sends 1 email

If you hit the limit, consider:
1. Using ESP32 SMTP (unlimited)
2. Adding cooldown timer to reduce frequency
3. Only sending emails for critical alerts (fence current, not RFID denials)

---

## Security Recommendations

### EmailJS (Frontend)
- ✅ Safe to commit User ID and Service ID to GitHub
- ✅ EmailJS validates requests server-side
- ✅ No passwords stored in code

### ESP32 SMTP (Backend)
- ❌ **DO NOT** commit email credentials to GitHub
- ✅ Store credentials in separate `secrets.h` file
- ✅ Add `secrets.h` to `.gitignore`

Example `secrets.h`:
```cpp
#define AUTHOR_EMAIL "your-email@gmail.com"
#define AUTHOR_PASSWORD "your-app-password"
```

---

## Next Steps

1. ✅ Follow EmailJS setup (Option 1)
2. ✅ Test email notifications
3. ✅ Customize warning colors/animations (optional)
4. ✅ Add sound alert (optional)
5. ✅ Commit changes to GitHub

**Note:** Don't forget to test the warning system by manually triggering your capacitive sensor! The warning popup should appear immediately, and you should receive an email within 1-2 seconds.

---

## Support

If you encounter issues:
1. Check browser console (F12) for errors
2. Check EmailJS dashboard for delivery status
3. Verify ESP32 is sending correct data format
4. Test with a simple alert: `showWarning("Test", "This is a test", "Details")`

Created by: GitHub Copilot  
Last Updated: 2024
