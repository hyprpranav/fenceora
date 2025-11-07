# 🚀 5-Minute Email Setup - Quick Start

## Your Configuration

**SENDER EMAIL:** harishpranavs259@gmail.com  
**PURPOSE:** Sends alerts when fence current, tampering, or unauthorized access detected

---

## ⚡ Quick Steps (Do This Now!)

### 1️⃣ Get Gmail App Password (2 minutes)

**Go to:** https://myaccount.google.com/security

**Enable 2-Step Verification:**
- Click "2-Step Verification"
- Follow setup (verify with phone)

**Create App Password:**
- Go to: https://myaccount.google.com/apppasswords
- App: **Mail**
- Device: **Other** → Type "Fenceora"
- Click **Generate**
- **COPY THE 16-CHARACTER PASSWORD!**

Example: `xqzy mkjh pqrs tuvw` → Remove spaces → `xqzymkjhpqrstuvw`

---

### 2️⃣ Update ESP32 Code (2 minutes)

Open: `Fenceora_V3_FIXED.ino`

**Find line 52 and paste your App Password:**

```cpp
// BEFORE:
#define SENDER_PASSWORD "YOUR_GMAIL_APP_PASSWORD"

// AFTER:
#define SENDER_PASSWORD "xqzymkjhpqrstuvw"  // Your actual password (no spaces!)
```

**Find lines 57-62 and add receiver emails:**

```cpp
// BEFORE:
const char* receiverEmails[] = {
  "receiver1@gmail.com",
  "receiver2@gmail.com",
  "receiver3@gmail.com",
  "receiver4@gmail.com"
};
const int numReceivers = 4;

// AFTER (example with 2 receivers):
const char* receiverEmails[] = {
  "yourfriend@gmail.com",    // Who should get alerts?
  "yourfamily@gmail.com"     // Add 1-4 emails
};
const int numReceivers = 2;  // Change to match number of emails
```

---

### 3️⃣ Upload & Test (1 minute)

1. **Save** the file
2. **Upload** to ESP32 (click → button)
3. **Open Serial Monitor** (115200 baud)
4. **Wait for:** `✓ WiFi Connected!`
5. **Trigger sensor** (touch fence wire)
6. **Look for:**
   ```
   📧 Sending email notification...
   ✅ Email sent successfully!
   ```
7. **Check inbox** of all receiver emails

---

## ✅ Done!

When alerts happen, receivers will get emails like this:

```
━━━━━━━━━━━━━━━━━━━━━━━━━
   ⚠️ CRITICAL ALERT
━━━━━━━━━━━━━━━━━━━━━━━━━

Fence Current Detected

Message: Electric field detected 
on perimeter fence!

Details: Voltage: 12.5V | 
Sensor: Capacitive

Timestamp: 2024-11-07 14:35:22
━━━━━━━━━━━━━━━━━━━━━━━━━
```

---

## 🆘 Problems?

**"Email notifications not configured"**  
→ App Password still says `YOUR_GMAIL_APP_PASSWORD`  
→ Paste your actual 16-character password

**"SMTP connection failed"**  
→ App Password is wrong  
→ 2-Step Verification not enabled  
→ WiFi not connected

**No email received**  
→ Check **spam folder**  
→ Check receiver email addresses (typos?)  
→ Serial Monitor says "✅ Email sent"?

---

## 📖 Need More Help?

Read full guides:
- **GMAIL_SETUP_INSTRUCTIONS.md** (detailed troubleshooting)
- **EMAIL_SYSTEM_SUMMARY.md** (features & configuration)

---

**That's it!** Just 3 steps and you'll have email notifications working! 🎉
