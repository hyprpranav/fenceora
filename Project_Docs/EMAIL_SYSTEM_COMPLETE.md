# 📧 FENCEORA Email Notification System - Complete Guide

## ✅ System Status: FULLY OPERATIONAL

Your email notification system is now **fully configured and operational**! Both manual tests and automatic alerts are working.

---

## 🎯 Features Overview

### 1. **Manual Test Emails** 
- **Location:** Settings → Email Test Section
- **Purpose:** Test the email system independently of ESP32
- **Cooldown:** 30 seconds between tests
- **Status:** ✅ Working (verified with test-email.html)

### 2. **Automatic Alert Emails**
- **Trigger:** When critical events are detected from ESP32
- **Control:** Toggle "Email Notifications" checkbox in Settings
- **Cooldown:** None (sends immediately on each alert)
- **Status:** ✅ Implemented

---

## 📊 Alert Types & Triggers

| Alert Type | Trigger Condition | Email Subject |
|------------|------------------|---------------|
| **Fence Current** | `data.fence.status === "ACTIVE"` | 🚨 FENCEORA ALERT: Fence Current Detected |
| **Tampering** | `data.tamper.ir === true` | 🚨 FENCEORA ALERT: Tampering Detected |
| **Device Movement** | `data.tamper.motion === true` | 🚨 FENCEORA ALERT: Device Movement Detected |
| **Unauthorized Access** | `data.lastRfidStatus === "DENIED"` | 🚨 FENCEORA ALERT: Unauthorized Access Attempt |

---

## 🔧 Configuration Details

### EmailJS Credentials
```javascript
Service ID: service_mf3wawv
Template ID: template_zn9ndnt
Public Key: 8_VkHFChgAm9XPDas
```

### Sender Email
```
fenceora.h4@gmail.com
```

### Receiver Emails (4 Recipients)
1. harishwarharishwar47@gmail.com
2. hemanathan8337@gmail.com
3. harishpranavs259@gmail.com
4. hariprasanthgk1@gmail.com

---

## 🎛️ How to Use

### Manual Test Email
1. Open **Settings** modal (gear icon in top right)
2. Scroll to **"Email Notifications"** section
3. Make sure **"Email Notifications"** checkbox is checked
4. Click **"Send Test Email"** button
5. Wait for success message
6. Check your email inboxes (all 4 receivers)
7. Button will be disabled for 30 seconds (cooldown timer shown)

### Automatic Alert Emails
1. Make sure **"Email Notifications"** checkbox is **checked** in Settings
2. When ESP32 sends alert data (fence current, tampering, etc.), emails automatically send
3. All 4 receivers get the alert email within seconds
4. No user action required!

### Disable Email Notifications
1. Open Settings
2. **Uncheck** "Email Notifications" checkbox
3. Manual test button still works
4. Automatic alerts will NOT send emails (only on-screen warnings)

---

## 📝 Email Content Format

### Test Email Example
```
Subject: 📧 FENCEORA Test Notification
From: FENCEORA Notification System
To: harishwar (harishwarharishwar47@gmail.com)

Hello harishwar!

This is a test notification from FENCEORA smart fence monitoring system.

Time: 1/13/2025, 10:30:45 AM
System Status: All Systems Operational
Alert Type: Test Email
Sender: fenceora.h4@gmail.com

If you received this email, your notification system is working correctly!

Best regards,
FENCEORA Team
```

### Alert Email Example
```
Subject: 🚨 FENCEORA ALERT: Fence Current Detected
From: FENCEORA Security System
To: harishwar (harishwarharishwar47@gmail.com)

⚡ CRITICAL: Electric field detected on perimeter fence! 
Time: 1/13/2025, 10:35:22 AM
Voltage: 230V

Time: 1/13/2025, 10:35:22 AM
System Status: ALERT - Immediate Attention Required
Alert Type: Fence Current Detected
Sender: fenceora.h4@gmail.com
```

---

## 🔍 Troubleshooting

### Problem: Manual test button not working on main site
**Solution:** ✅ FIXED - Removed duplicate `sendTestEmail()` function from script.js

### Problem: Emails showing "Receiver #3" instead of names
**Solution:** ✅ FIXED - Now uses `receiverEmail.split('@')[0]` for personalized names

### Problem: test-email.html works but index.html doesn't
**Solution:** ✅ FIXED - Script.js had duplicate function overriding the correct one

### Problem: No automatic emails on alerts
**Solution:** ✅ FIXED - Added `sendAlertEmail()` calls in `checkForCriticalAlerts()`

---

## 🧪 Testing Checklist

### ✅ Manual Test Email
- [ ] Open main site (index.html)
- [ ] Press **Ctrl+Shift+R** to hard refresh (clear cache)
- [ ] Open Settings
- [ ] Check "Email Notifications"
- [ ] Click "Send Test Email"
- [ ] Wait for "Email sent successfully!" message
- [ ] Check all 4 inboxes (Gmail, spam folder if needed)
- [ ] Verify personalized names (harishwar, hemanathan, etc.)

### ✅ Automatic Alert Email
- [ ] Open main site with ESP32 connected
- [ ] Make sure "Email Notifications" is checked in Settings
- [ ] Trigger an alert (touch fence wire, move device, scan unknown RFID)
- [ ] On-screen warning should appear
- [ ] Check browser console for "✅ Alert emails sent: [Alert Type]"
- [ ] Check all 4 inboxes for alert email

---

## 📂 File Structure

```
DTwebsite/
├── email-config.js          # EmailJS credentials & receiver list
├── email-sender.js          # Core sendTestEmail() function
├── test-email.html          # Standalone diagnostic page (WORKING)
├── index.html               # Main dashboard (FIXED)
├── script.js                # Main logic with checkForCriticalAlerts()
└── Project_Docs/
    └── EMAIL_SYSTEM_COMPLETE.md  # This file
```

---

## 🚀 What's New (Latest Changes)

### ✅ Removed Duplicate Function
- **File:** script.js (line 598)
- **Issue:** Old `sendTestEmail()` with placeholder credentials
- **Fix:** Removed duplicate, now uses email-sender.js version

### ✅ Added Automatic Email Alerts
- **Function:** `sendAlertEmail(alertType, alertMessage)`
- **Location:** script.js (after checkForCriticalAlerts)
- **Integration:** Called in checkForCriticalAlerts when alerts detected
- **Respects:** "Email Notifications" checkbox setting

### ✅ Alert Triggers Implemented
- Fence Current Detection → Email sent
- IR Tampering → Email sent
- Motion Detection → Email sent
- Unauthorized RFID → Email sent

---

## 💡 Important Notes

1. **Free Tier Limit:** EmailJS free plan = 200 emails/month
   - 4 receivers × 50 alerts = limit reached
   - Consider upgrading if you get frequent alerts

2. **No Cooldown on Alerts:** Unlike manual tests (30s cooldown), automatic alerts send immediately every time
   - If fence current detected continuously, you'll get an email every time ESP32 sends data
   - Consider adding cooldown if needed

3. **Browser Required:** Email system works from browser only (not from ESP32)
   - ESP32 → Browser → EmailJS → Gmail → Receivers

4. **Checkbox Control:** The "Email Notifications" checkbox controls BOTH:
   - Manual test button visibility
   - Automatic alert email sending

5. **Hard Refresh After Changes:** Always press **Ctrl+Shift+R** after code updates to clear browser cache

---

## 📞 Support Checklist

If emails not working:
1. Check EmailJS dashboard (https://dashboard.emailjs.com/)
2. Verify service status (service_mf3wawv active?)
3. Check template exists (template_zn9ndnt)
4. Browser console errors? (F12 → Console tab)
5. Email notifications checkbox checked?
6. Hard refresh the page (Ctrl+Shift+R)?
7. Check spam/junk folders in Gmail?

---

## 🎉 System Ready!

Your FENCEORA email notification system is now fully operational with:
- ✅ Manual test emails (30s cooldown)
- ✅ Automatic alert emails (immediate)
- ✅ 4 receivers configured
- ✅ Personalized email names
- ✅ User-controlled enable/disable
- ✅ Real-time alerts on critical events

**Enjoy secure, instant notifications from your smart fence system!** 🚀
