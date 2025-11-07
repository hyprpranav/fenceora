# 📧 EMAIL TEST NOTIFICATION SETUP GUIDE

## ✅ **IMPORTANT: Works WITHOUT ESP32 Connection!**

The test email feature is **completely independent** of your ESP32. You can test email notifications directly from your browser without needing to connect to ESP32.

---

## 🎯 **Quick Start (Test Mode)**

1. **Open your website** in any browser
2. **Click Settings icon** (⚙️) in the top-right corner
3. **Email Notifications checkbox** is already checked
4. **Scroll down** to see the "Test Email Notification" section
5. **Click "Send Test Email"** button
6. ✅ **Success message** appears + check browser console for details
7. ⏱️ **Wait 30 seconds** before sending next test

**Current Status:** SIMULATION MODE  
(Shows success but doesn't send real emails yet)

---

## 🚀 **Send REAL Emails (Setup Required)**

To send actual emails to your 4 receivers, follow these steps:

### **Step 1: Create EmailJS Account** (FREE)

1. Go to [https://www.emailjs.com/](https://www.emailjs.com/)
2. Click "**Sign Up**" and create a free account
3. Verify your email address

### **Step 2: Add Email Service**

1. In EmailJS Dashboard, click "**Email Services**"
2. Click "**Add New Service**"
3. Select "**Gmail**" (recommended)
4. Click "**Connect Account**" and sign in with your sender Gmail
5. Give it a name like "FENCEORA_Gmail"
6. Click "**Create Service**"
7. **Copy the Service ID** (e.g., `service_abc1234`)

### **Step 3: Create Email Template**

1. In EmailJS Dashboard, click "**Email Templates**"
2. Click "**Create New Template**"
3. **Template Name:** `FENCEORA_Test_Notification`
4. **Template Content:**

```
Subject: {{subject}}

Hello {{to_name}},

{{message}}

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Alert Type: {{alert_type}}
Timestamp: {{timestamp}}
System Status: {{system_status}}
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

This is an automated message from the FENCEORA Security System.

Best regards,
{{from_name}}
```

5. In "**Settings**" tab, add these recipient emails:
   - Add all 4 receiver email addresses
6. Click "**Save**"
7. **Copy the Template ID** (e.g., `template_xyz5678`)

### **Step 4: Get Public Key**

1. In EmailJS Dashboard, click "**Account**" (top-right)
2. Look for "**Public Key**" or "**API Keys**"
3. **Copy your Public Key** (e.g., `user_abcdefg123456`)

### **Step 5: Configure Your Website**

1. Open `email-config.js` file in your website folder
2. Replace the placeholder values:

```javascript
const EMAIL_CONFIG = {
  emailjs: {
    enabled: true,  // ⚠️ Change this to true!
    serviceId: 'service_abc1234',    // Your Service ID
    templateId: 'template_xyz5678',  // Your Template ID
    publicKey: 'user_abcdefg123456'  // Your Public Key
  },
  
  recipients: {
    sender: 'your-fenceora-system@gmail.com',  // Update this
    receivers: [
      'receiver1@gmail.com',  // Update with real emails
      'receiver2@gmail.com',
      'receiver3@gmail.com',
      'receiver4@gmail.com'
    ]
  }
};
```

3. **Save the file**

### **Step 6: Test Real Emails!**

1. **Refresh your website**
2. Open **Settings** → **Email Test Section**
3. Click "**Send Test Email**"
4. ✅ Check your **4 receiver email inboxes**!
5. 📧 You should receive real test emails

---

## 🎨 **Features**

✅ **No ESP32 Required** - Works directly from browser  
✅ **30-Second Cooldown** - Prevents spam  
✅ **Visual Timer** - Shows countdown  
✅ **Success/Error Messages** - Clear feedback  
✅ **Console Logging** - Detailed debug info  
✅ **Beautiful UI** - Modern design with animations  
✅ **Both Themes** - Works in Dark & Light mode  

---

## 📝 **How It Works**

```
User clicks "Send Test Email"
         ↓
Button shows loading spinner
         ↓
JavaScript reads email-config.js
         ↓
If EmailJS configured:
  → Sends REAL email via EmailJS
  → Email goes to all 4 receivers
         ↓
Else (simulation mode):
  → Shows success in console
  → No actual email sent
         ↓
30-second timer starts
         ↓
Button re-enables after 30 seconds
```

---

## 🔧 **Troubleshooting**

### **"Email sending failed" error:**
- Check your EmailJS Service ID, Template ID, and Public Key
- Make sure `enabled: true` in email-config.js
- Verify your Gmail account is connected in EmailJS

### **Emails not received:**
- Check spam/junk folder
- Verify receiver email addresses in email-config.js
- Check EmailJS dashboard for sent email history
- Make sure template has recipient emails added

### **Button not visible:**
- Make sure "Email Notifications" checkbox is checked
- Refresh the page
- Open browser console (F12) for errors

---

## 📧 **Email System Architecture**

```
FENCEORA Website (Browser)
         ↓
   email-sender.js
         ↓
   EmailJS Service (Cloud)
         ↓
   Gmail SMTP
         ↓
4 Receiver Emails
```

**No ESP32 in the loop!** This is purely browser → cloud → email.

---

## 💡 **Pro Tips**

1. **Free Tier:** EmailJS free plan allows 200 emails/month
2. **Multiple Receivers:** Add all 4 emails in the template settings
3. **Custom Templates:** Modify the email template for different alert types
4. **Testing:** Use simulation mode first, then switch to real emails
5. **Security:** Never commit your real API keys to GitHub

---

## 📞 **Need Help?**

- EmailJS Documentation: [https://www.emailjs.com/docs/](https://www.emailjs.com/docs/)
- EmailJS Support: [https://www.emailjs.com/support/](https://www.emailjs.com/support/)
- FENCEORA Team: fenceora.h4@gmail.com

---

**Last Updated:** November 7, 2025  
**Version:** 1.0.0  
**Status:** ✅ Fully Functional (Independent of ESP32)
