# 📧 Gmail Email Notification Setup Guide

## Overview

Your Fenceora system is now configured to send email notifications **directly from the ESP32** when critical events occur:

- ⚡ **Fence current detected** (capacitive sensor)
- 🚨 **Tampering detected** (IR sensor)
- ⚠️ **Device movement** (accelerometer)
- 🔒 **Unauthorized RFID card** scanned

### Email Configuration:
- **SENDER:** harishpranavs259@gmail.com (sends all alerts)
- **RECEIVERS:** Up to 4 emails you will configure (receive all alerts)

---

## ⚙️ Step 1: Get Gmail App Password

Google requires an "App Password" (not your regular password) for third-party apps to send emails.

### Instructions:

1. **Enable 2-Step Verification** (required first):
   - Go to: https://myaccount.google.com/security
   - Sign in with **harishpranavs259@gmail.com**
   - Find **"2-Step Verification"** section
   - Click **"Get Started"** and follow the setup
   - Verify with your phone number

2. **Create App Password**:
   - After enabling 2-Step Verification, go to: https://myaccount.google.com/apppasswords
   - You may need to sign in again
   - Under **"Select app"**, choose: **"Mail"**
   - Under **"Select device"**, choose: **"Other (Custom name)"**
   - Type: **"Fenceora ESP32"**
   - Click **"Generate"**

3. **Copy the App Password**:
   - Google will show a **16-character password** like: `abcd efgh ijkl mnop`
   - **IMPORTANT:** Copy this password (remove spaces: `abcdefghijklmnop`)
   - You won't be able to see it again!

---

## 📝 Step 2: Configure ESP32 Code

Open `Fenceora_V3_FIXED.ino` and find these lines (around line 48-68):

### Current Configuration:
```cpp
// === Email Configuration (Gmail SMTP) ===
#define SENDER_EMAIL "harishpranavs259@gmail.com"
#define SENDER_PASSWORD "YOUR_GMAIL_APP_PASSWORD"  // ⚠️ REPLACE THIS

// RECEIVER EMAILS (People who will receive notifications)
const char* receiverEmails[] = {
  "receiver1@gmail.com",     // Replace with 1st receiver email
  "receiver2@gmail.com",     // Replace with 2nd receiver email  
  "receiver3@gmail.com",     // Replace with 3rd receiver email
  "receiver4@gmail.com"      // Replace with 4th receiver email
};
const int numReceivers = 4;  // Change this if you have fewer receivers
```

### What You Need to Replace:

#### 1. Set Your App Password:
Replace `YOUR_GMAIL_APP_PASSWORD` with the 16-character password from Step 1:

```cpp
#define SENDER_PASSWORD "abcdefghijklmnop"  // Your actual App Password (no spaces)
```

**Example:**
```cpp
#define SENDER_PASSWORD "xqzy mkjh pqrs tuvw"  // If Google gave you this
// Remove spaces:
#define SENDER_PASSWORD "xqzymkjhpqrstuvw"    // Use this in code
```

#### 2. Set Receiver Emails:
Replace the placeholder emails with **actual email addresses** of people who should receive alerts:

**Example with 4 receivers:**
```cpp
const char* receiverEmails[] = {
  "admin@gmail.com",           // Main admin
  "security@company.com",      // Security team
  "owner@gmail.com",           // Owner
  "backup@gmail.com"           // Backup contact
};
const int numReceivers = 4;
```

**Example with 2 receivers:**
```cpp
const char* receiverEmails[] = {
  "admin@gmail.com",           // Main admin
  "owner@gmail.com"            // Owner
};
const int numReceivers = 2;  // Changed from 4 to 2
```

**Example with 1 receiver:**
```cpp
const char* receiverEmails[] = {
  "admin@gmail.com"            // Single receiver
};
const int numReceivers = 1;  // Changed from 4 to 1
```

---

## 🔧 Step 3: Upload Code to ESP32

1. **Save the file** after making changes
2. **Connect ESP32** to your computer via USB
3. **Upload the code:**
   - Click **Upload** button in Arduino IDE (→ icon)
   - Wait for "Done uploading" message
4. **Open Serial Monitor** (115200 baud)
5. **Check for confirmation:**
   ```
   ✓ WiFi Connected!
   ✓ IP Address: 192.168.x.x
   ```

---

## 🧪 Step 4: Test Email Notifications

### Test 1: Trigger Fence Current
1. Touch the fence wire (trigger capacitive sensor)
2. **Expected Serial Monitor output:**
   ```
   ⚡ FENCE ELECTRIC FIELD DETECTED!
   
   📧 Sending email notification...
      → Adding receiver: admin@gmail.com
      → Adding receiver: security@company.com
      → Adding receiver: owner@gmail.com
      → Adding receiver: backup@gmail.com
   ✅ Email sent successfully to all receivers!
   ```

3. **Check your inbox** (all receiver emails)
4. **You should receive:**
   - Subject: `🚨 FENCEORA ALERT: Fence Current Detected`
   - HTML formatted email with red gradient header
   - Details: voltage, timestamp, sensor type

### Test 2: Trigger Tampering
1. Hold your hand near the IR sensor
2. **Expected:** Email with subject "Tampering Detected"

### Test 3: Trigger Movement
1. Shake or move the ESP32 device
2. **Expected:** Email with subject "Device Movement Detected"

### Test 4: Unauthorized RFID
1. Scan an unknown RFID card
2. **Expected:** Email with subject "Unauthorized RFID Access Attempt"

---

## 🔒 Security Recommendations

### ✅ DO:
- Keep your App Password **secret** (treat it like a password)
- Use 2-Step Verification on your Gmail account
- Test email notifications regularly
- Check spam folder if emails don't arrive

### ❌ DON'T:
- **Never commit Gmail App Password to GitHub!**
- Don't share your App Password with others
- Don't use your regular Gmail password in code
- Don't disable 2-Step Verification (emails will stop working)

### 🔐 How to Keep Password Safe:

**Option 1: Use a separate config file**
Create a file called `secrets.h`:
```cpp
// secrets.h
#define SENDER_PASSWORD "your_app_password_here"
```

Then in `Fenceora_V3_FIXED.ino`:
```cpp
#include "secrets.h"  // Load password from separate file
```

Add `secrets.h` to `.gitignore`:
```
secrets.h
```

**Option 2: Environment variables** (advanced)
Use Arduino IDE's build flags to pass password at compile time.

---

## 📊 Email Cooldown System

To prevent **email spam**, the system has a **30-second cooldown**:

- If fence current is detected → Email sent
- If fence current is detected again **within 30 seconds** → Email skipped
- After 30 seconds → Email will be sent again

**Why?** If the capacitive sensor is triggered continuously, you don't want 100 emails per minute!

**To change cooldown period**, edit this line (around line 66):
```cpp
const unsigned long EMAIL_COOLDOWN = 30000; // 30 seconds (30,000 milliseconds)
```

Examples:
- 1 minute: `60000`
- 5 minutes: `300000`
- 10 seconds: `10000`

---

## 📧 Email Format Preview

When an alert is triggered, receivers will get an HTML email like this:

```
┌─────────────────────────────────────┐
│  ⚠️ CRITICAL ALERT                  │  (Red gradient background)
└─────────────────────────────────────┘

Fence Current Detected

Message: Electric field detected on perimeter fence!

Details: Voltage: 12.5V | Sensor: Capacitive

Timestamp: 2024-11-07 14:35:22

─────────────────────────────────────

Please check your Fenceora dashboard immediately.

This is an automated alert from your Fenceora Smart Fence System.
```

---

## ❓ Troubleshooting

### Issue: "Email notifications not configured"
**Solution:**
- Check that `SENDER_PASSWORD` is not `YOUR_GMAIL_APP_PASSWORD`
- Paste your actual 16-character App Password from Google

### Issue: "SMTP connection failed"
**Causes:**
1. **Wrong App Password** → Double-check the password (no spaces)
2. **2-Step Verification not enabled** → Enable it in Google Account settings
3. **ESP32 not connected to WiFi** → Check WiFi credentials
4. **Gmail blocked the login** → Check your Gmail for "Critical security alert" email and allow the app

**Solution:**
- Go to: https://myaccount.google.com/notifications
- If you see a blocked sign-in attempt, click "Yes, it was me"
- Try again

### Issue: "Email send failed"
**Check Serial Monitor for error message:**
- `Error: authentication failed` → App Password is wrong
- `Error: connection timeout` → WiFi issue or SMTP server unreachable
- `Error: invalid recipient` → Check receiver email addresses (typos?)

### Issue: Emails not received
**Check:**
1. **Spam folder** → Gmail might mark automated emails as spam
2. **Receiver email addresses** → Make sure they're correct (no typos)
3. **Serial Monitor** → Does it say "✅ Email sent successfully"?
4. **Gmail sent folder** → Check if email appears in harishpranavs259@gmail.com sent folder

### Issue: Too many emails (spam)
**Solution:** Increase cooldown period:
```cpp
const unsigned long EMAIL_COOLDOWN = 60000; // 1 minute instead of 30 seconds
```

---

## 📋 Quick Checklist

Before uploading code, verify:

- [ ] Gmail 2-Step Verification enabled
- [ ] App Password generated and copied (16 characters, no spaces)
- [ ] `SENDER_PASSWORD` updated in code with App Password
- [ ] `receiverEmails[]` array filled with actual email addresses
- [ ] `numReceivers` set to correct number (1, 2, 3, or 4)
- [ ] WiFi credentials correct (`ssid` and `password`)
- [ ] Code uploaded to ESP32 successfully
- [ ] Serial Monitor shows "✓ WiFi Connected!"

After upload, test:

- [ ] Trigger fence current → Email received by all receivers
- [ ] Trigger IR sensor → Email received
- [ ] Move device → Email received
- [ ] Scan unauthorized RFID → Email received
- [ ] Check Serial Monitor for "✅ Email sent successfully"

---

## 🔧 Advanced: Multiple Sender Emails

If you want different alerts from different sender emails:

**Example:**
- Fence alerts from: `fencealert@gmail.com`
- Tamper alerts from: `securityalert@gmail.com`

**Solution:** Create multiple `sendEmailNotification()` functions with different sender credentials.

---

## 📚 Library Required

Make sure **ESP Mail Client** library is installed:

1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search: **"ESP Mail Client"**
4. Install: **"ESP Mail Client by Mobizt"** (version 3.4.16 or newer)
5. Click **Install**

**Dependencies** (auto-installed):
- ESP32 Arduino Core v3.x
- WiFi (built-in)
- ArduinoJson v7.x

---

## 🎯 Summary

**What happens when an alert occurs:**

1. **ESP32 detects event** (fence current, tampering, etc.)
2. **Checks cooldown** (30 seconds since last email)
3. **Connects to Gmail SMTP** server (smtp.gmail.com:465)
4. **Authenticates** with harishpranavs259@gmail.com + App Password
5. **Sends HTML email** to all receivers
6. **Serial Monitor** shows success/failure message
7. **Dashboard** also shows warning popup (if connected)

**Benefits:**
- ✅ No EmailJS account needed
- ✅ No browser dependency (works 24/7)
- ✅ Works even when dashboard is closed
- ✅ Multiple receivers get notified
- ✅ Professional HTML emails
- ✅ 30-second spam prevention
- ✅ Free (no email service fees)

---

## 📞 Need Help?

If you encounter issues:

1. **Check Serial Monitor** (115200 baud) for error messages
2. **Test WiFi connection** first (should show IP address)
3. **Verify App Password** (most common issue)
4. **Check Gmail account** for blocked sign-in alerts
5. **Test with 1 receiver** first before adding more

---

**Created by:** GitHub Copilot  
**Last Updated:** November 7, 2024  
**Sender Email:** harishpranavs259@gmail.com  
**Status:** ✅ Ready to configure (just need App Password and receiver emails)
