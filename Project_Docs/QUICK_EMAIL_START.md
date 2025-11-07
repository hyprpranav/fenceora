# ⚡ QUICK START - Send Emails in 5 Minutes!

## 🎯 **Your Mission:** Get emails working NOW!

---

## 🚀 **Option 1: Quick Setup with EmailJS (Recommended)**

### **1. Create Account (2 min)**
- Go to: https://www.emailjs.com/
- Click "Sign Up" → Sign in with Google
- ✅ Done!

### **2. Connect Gmail (1 min)**
- Click "Email Services" → "Add New Service"
- Choose "Gmail" → Connect `fenceora.h4@gmail.com`
- **COPY the Service ID** (save it!)

### **3. Create Template (2 min)**
- Click "Email Templates" → "Create New Template"
- **To:** `{{to_email}}`
- **Subject:** `{{subject}}`
- **Body:** `{{message}}`
- **COPY the Template ID** (save it!)

### **4. Get Public Key (30 sec)**
- Click your profile → "Account"
- **COPY your Public Key** (save it!)

### **5. Update Config (1 min)**
- Open `email-config.js`
- Replace:
  - `serviceId: 'YOUR_SERVICE_ID_HERE'` → paste from step 2
  - `templateId: 'YOUR_TEMPLATE_ID_HERE'` → paste from step 3
  - `publicKey: 'YOUR_PUBLIC_KEY_HERE'` → paste from step 4
- Save file

### **6. Test! (30 sec)**
- Refresh website (Ctrl+F5)
- Click Settings → "Send Test Email"
- ✅ Check all 4 inboxes! (check spam too!)

---

## 📧 **Current Email Receivers:**

1. `hyprpranav@gmail.com`
2. `dhyanarayan2005@gmail.com`
3. `ganeshrpms2022@gmail.com`
4. `haran90705@gmail.com`

**Sender:** `fenceora.h4@gmail.com`

---

## ⚠️ **IMPORTANT NOTES:**

1. **First time?** Emails might go to spam folder!
2. **30 second cooldown** between tests
3. **Free tier:** 200 emails/month
4. **Check console** (F12) for detailed logs
5. **No ESP32 needed** - works from browser!

---

## 🔧 **If Emails Don't Arrive:**

1. ✅ Check SPAM/JUNK folder first!
2. ✅ Wait 1-2 minutes for delivery
3. ✅ Check EmailJS dashboard → "Logs" tab
4. ✅ Verify all 3 IDs are correct in email-config.js
5. ✅ Make sure Gmail service is "Active" in EmailJS

---

## 📝 **Email Template for Step 3:**

```
To: {{to_email}}
Subject: {{subject}}

Hello {{to_name}},

{{message}}

Alert Type: {{alert_type}}
Timestamp: {{timestamp}}
System Status: {{system_status}}
Receiver: #{{receiver_number}}

Best regards,
{{from_name}}
```

---

## 🎉 **Success Looks Like:**

- Green success message in website
- Console shows: "✅ ALL EMAILS SENT SUCCESSFULLY!"
- All 4 receivers get email within 1-2 minutes
- 30-second timer starts counting down

---

## 📞 **Need Help Right Now?**

1. Read: `EMAIL_SETUP_NOW.md` (detailed guide)
2. EmailJS Docs: https://www.emailjs.com/docs/
3. Video Tutorial: https://www.emailjs.com/docs/tutorial/overview/

---

**⏱️ Total Time: 5-7 minutes**  
**💰 Cost: FREE (200 emails/month)**  
**🚀 Speed: 2-3 seconds per test**

---

**READY? GO!** 🏃‍♂️💨
